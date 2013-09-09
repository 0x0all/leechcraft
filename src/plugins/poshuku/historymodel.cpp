/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "historymodel.h"
#include <algorithm>
#include <QTimer>
#include <QVariant>
#include <QAction>
#include <QtDebug>
#include <util/models/treeitem.h>
#include <util/defaulthookproxy.h>
#include <interfaces/core/icoreproxy.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "poshuku.h"

namespace LeechCraft
{
namespace Poshuku
{
	using LeechCraft::Util::TreeItem;

	namespace
	{
		/** Returns the number of the section for the given date.
			*
			* - Today
			* - Yesterday
			* - Two days ago
			* - Last week
			* - Last month
			* - Last 2 months
			* - Last 3 months
			* - Last 4 months
			* - ...
			* - Last N months
			*/
		int SectionNumber (const QDateTime& date)
		{
			QDateTime current = QDateTime::currentDateTime ();
			QDate orig = current.date ();
			if (date.daysTo (current) == 0)
				return 0;
			else if (date.daysTo (current) == 1)
				return 1;
			else if (date.daysTo (current) == 2)
				return 2;
			else if (date.daysTo (current) <= 7)
				return 3;

			int i = 0;
			while (true)
			{
				current.setDate (orig.addMonths (--i));

				if (date.daysTo (current) <= 0)
					return -i + 3;
			}
		}

		QString SectionName (int number)
		{
			switch (number)
			{
				case 0:
					return QObject::tr ("Today");
				case 1:
					return QObject::tr ("Yesterday");
				case 2:
					return QObject::tr ("Two days ago");
				case 3:
					return QObject::tr ("Last week");
				case 4:
					return QObject::tr ("Last month");
				default:
					return QObject::tr ("Last %n month(s)", "", number - 3);
			}
		}
	};

	HistoryModel::HistoryModel (QObject *parent)
	: QAbstractItemModel (parent)
	{
		QList<QVariant> headers;
		headers << tr ("Title")
			<< tr ("URL")
			<< tr ("Date");
		QTimer::singleShot (0,
				this,
				SLOT (loadData ()));
		RootItem_ = new TreeItem (headers);

		GarbageTimer_ = new QTimer (this);
		GarbageTimer_->start (15 * 60 * 1000);
		connect (GarbageTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (collectGarbage ()));
	}

	HistoryModel::~HistoryModel ()
	{
		delete RootItem_;
	}

	int HistoryModel::columnCount (const QModelIndex& index) const
	{
		if (index.isValid ())
			return static_cast<TreeItem*> (index.internalPointer ())->ColumnCount ();
		else
			return RootItem_->ColumnCount ();
	}

	QVariant HistoryModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return QVariant ();

		return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column (), role);
	}

	Qt::ItemFlags HistoryModel::flags (const QModelIndex&) const
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant HistoryModel::headerData (int h, Qt::Orientation orient,
			int role) const
	{
		if (orient == Qt::Horizontal && role == Qt::DisplayRole)
			return RootItem_->Data (h);

		return QVariant ();
	}

	QModelIndex HistoryModel::index (int row, int col,
			const QModelIndex& parent) const
	{
		if (!hasIndex (row, col, parent))
			return QModelIndex ();

		TreeItem *parentItem;

		if (!parent.isValid ())
			parentItem = RootItem_;
		else
			parentItem = static_cast<TreeItem*> (parent.internalPointer ());

		TreeItem *childItem = parentItem->Child (row);
		if (childItem)
			return createIndex (row, col, childItem);
		else
			return QModelIndex ();
	}

	QModelIndex HistoryModel::parent (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return QModelIndex ();

		TreeItem *childItem = static_cast<TreeItem*> (index.internalPointer ()),
					*parentItem = childItem->Parent ();

		if (parentItem == RootItem_)
			return QModelIndex ();

		return createIndex (parentItem->Row (), 0, parentItem);
	}

	int HistoryModel::rowCount (const QModelIndex& parent) const
	{
		TreeItem *parentItem;
		if (parent.column () > 0)
			return 0;

		if (!parent.isValid ())
			parentItem = RootItem_;
		else
			parentItem = static_cast<TreeItem*> (parent.internalPointer ());

		return parentItem->ChildCount ();
	}

	void HistoryModel::addItem (QString title, QString url,
			QDateTime date, QObject *browserWidget)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookAddingToHistory (proxy, title, url, date, browserWidget);
		if (proxy->IsCancelled ())
			return;

		QVariantList result = proxy->GetReturnValue ().toList ();
		int size = result.size ();
		if (size >= 1)
			title = result.at (0).toString ();
		if (size >= 2)
			url = result.at (1).toString ();
		if (size >= 3)
			date = result.at (2).toDateTime ();

		HistoryItem item =
		{
			title,
			date,
			url
		};
		Core::Instance ().GetStorageBackend ()->AddToHistory (item);
	}

	QList<QMap<QString, QVariant>> HistoryModel::getItemsMap () const
	{
		QList<QMap<QString, QVariant>> result;
		Q_FOREACH (const HistoryItem& item, Items_)
		{
			QMap<QString, QVariant> map;
			map ["Title"] = item.Title_;
			map ["DateTime"] = item.DateTime_;
			map ["URL"] = item.URL_;
			result << map;
		}
		return result;
	}

	void HistoryModel::Add (const HistoryItem& item, bool announce)
	{
		int section = SectionNumber (item.DateTime_);

		while (section >= RootItem_->ChildCount ())
		{
			QList<QVariant> data;
			data << SectionName (RootItem_->ChildCount ())
				<< QString ("")
				<< QString ("");
			TreeItem *folder = new TreeItem (data, RootItem_);
			folder->ModifyData (0,
					Core::Instance ().GetProxy ()->GetIcon ("document-open-folder"),
					Qt::DecorationRole);
			RootItem_->AppendChild (folder);
		}

		QList<QVariant> data;
		data << item.Title_
			<< item.URL_
			<< item.DateTime_;

		TreeItem *folder = RootItem_->Child (section);

		TreeItem *thisItem = new TreeItem (data, RootItem_->Child (section));

		for (int i = folder->ChildCount () - 1; i >= 0; --i)
		{
			auto child = folder->Child (i);
			if (child->Data (ColumnURL) == item.URL_)
			{
				if (announce)
					beginRemoveRows (index (section, 0), i, i);
				folder->RemoveChild (i);
				if (announce)
					endRemoveRows ();
			}
		}

		if (announce)
			beginInsertRows (index (section, 0), 0, 0);
		folder->PrependChild (thisItem);
		if (announce)
			endInsertRows ();

		QIcon icon = Core::Instance ().GetIcon (QUrl (item.URL_));
		thisItem->ModifyData (0,
				icon, Qt::DecorationRole);
	}

	void HistoryModel::loadData ()
	{
		collectGarbage ();

		beginResetModel ();

		while (RootItem_->ChildCount ())
			RootItem_->RemoveChild (0);

		Items_.clear ();
		Core::Instance ().GetStorageBackend ()->LoadHistory (Items_);

		if (Items_.empty ())
			return;

		for (const auto& item : Items_)
			Add (item, false);

		endResetModel ();
	}

	void HistoryModel::handleItemAdded (const HistoryItem& item)
	{
		Items_.push_back (item);
		Add (item, true);
	}

	void HistoryModel::collectGarbage ()
	{
		int age = XmlSettingsManager::Instance ()->
			property ("HistoryClearOlderThan").toInt ();
		int maxItems = XmlSettingsManager::Instance ()->
			property ("HistoryKeepLessThan").toInt ();
		Core::Instance ().GetStorageBackend ()->ClearOldHistory (age, maxItems);
	}
}
}
