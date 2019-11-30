/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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

#include "favoritesmodel.h"
#include <algorithm>
#include <QTimer>
#include <QtDebug>
#include <QMimeData>
#include <QFileInfo>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <util/sll/prelude.h>
#include <util/sll/views.h>
#include <util/xpc/defaulthookproxy.h>
#include "filtermodel.h"
#include "core.h"
#include "editbookmarkdialog.h"

namespace LC
{
namespace Poshuku
{
	bool FavoritesModel::FavoritesItem::operator== (const FavoritesModel::FavoritesItem& item) const
	{
		return Title_ == item.Title_ &&
			URL_ == item.URL_ &&
			Tags_ == item.Tags_;
	}

	FavoritesModel::FavoritesModel (QObject *parent)
	: QAbstractItemModel (parent)
	{
		ItemHeaders_ << tr ("Title")
			<< tr ("URL")
			<< tr ("Tags");
	}

	void FavoritesModel::HandleStorageReady ()
	{
		loadData ();
	}

	int FavoritesModel::columnCount (const QModelIndex&) const
	{
		return ItemHeaders_.size ();
	}

	QVariant FavoritesModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return QVariant ();

		switch (role)
		{
		case Qt::DisplayRole:
			switch (index.column ())
			{
			case ColumnTitle:
				return Items_ [index.row ()].Title_;
			case ColumnURL:
				return Items_ [index.row ()].URL_;
			case ColumnTags:
				return Core::Instance ().GetProxy ()->GetTagsManager ()->Join (GetVisibleTags (index.row ()));
			default:
				return {};
			}
		case Qt::DecorationRole:
			if (index.column () == ColumnTitle)
				return Core::Instance ().GetIcon (Items_ [index.row ()].URL_);
			else
				return QVariant ();
		case Qt::ToolTipRole:
			return CheckResults_ [Items_ [index.row ()].URL_];
		case RoleTags:
			return Items_ [index.row ()].Tags_;
		default:
			return {};
		}
	}

	Qt::ItemFlags FavoritesModel::flags (const QModelIndex& index) const
	{
		Qt::ItemFlags result = Qt::ItemIsEnabled |
				Qt::ItemIsSelectable |
				Qt::ItemIsDragEnabled |
				Qt::ItemIsDropEnabled;
		if (index.column () == ColumnTags)
			result |= Qt::ItemIsEditable;
		return result;
	}

	QVariant FavoritesModel::headerData (int column, Qt::Orientation orient,
			int role) const
	{
		if (orient == Qt::Horizontal && role == Qt::DisplayRole)
			return ItemHeaders_.at (column);
		else
			return QVariant ();
	}

	QModelIndex FavoritesModel::index (int row, int column,
			const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return QModelIndex ();

		return createIndex (row, column);
	}

	QModelIndex FavoritesModel::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int FavoritesModel::rowCount (const QModelIndex& index) const
	{
		return index.isValid () ? 0 : Items_.size ();
	}

	bool FavoritesModel::setData (const QModelIndex& index,
			const QVariant& value, int)
	{
		switch (index.column ())
		{
		case ColumnTags:
		{
			const auto& userTags = value.toStringList ();
			const auto tm = Core::Instance ().GetProxy ()->GetTagsManager ();
			Items_ [index.row ()].Tags_ = tm->GetIDs (userTags);
			Core::Instance ().GetStorageBackend ()->UpdateFavorites (Items_ [index.row ()]);
			return true;
		}
		case ColumnTitle:
			Items_ [index.row ()].Title_ = value.toString ();
			Core::Instance ().GetStorageBackend ()->UpdateFavorites (Items_ [index.row ()]);
			return true;
		case ColumnURL:
			return true;
		default:
			return false;
		}
	}

	QModelIndex FavoritesModel::addItem (const QString& title,
			const QString& url, const QStringList& visibleTags)
	{
		const auto& tags = Core::Instance ().GetProxy ()->GetTagsManager ()->GetIDs (visibleTags);

		FavoritesItem item =
		{
			title,
			url,
			tags
		};

		try
		{
			Core::Instance ().GetStorageBackend ()->AddToFavorites (item);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
			return QModelIndex ();
		}

		auto proxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookAddedToFavorites (proxy, title, url, visibleTags);

		return createIndex (Items_.size () - 1, 0);
	}

	QList<QVariant> FavoritesModel::getItemsMap() const
	{
		const auto itm = Core::Instance ().GetProxy ()->GetTagsManager ();
		return Util::Map (Items_,
				[itm] (const auto& item) -> QVariant
				{
					return QVariantMap
					{
						{ "Title", item.Title_ },
						{ "URL", item.URL_ },
						{ "Tags", itm->GetTags (item.Tags_) }
					};
				});
	}

	Qt::DropActions FavoritesModel::supportedDropActions () const
	{
		return static_cast<Qt::DropActions> (Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);
	}

	QStringList FavoritesModel::mimeTypes () const
	{
		return { "text/uri-list" };
	}

	QMimeData* FavoritesModel::mimeData (const QModelIndexList& indexes) const
	{
		if (indexes.isEmpty ())
			return 0;

		QList<QUrl> urls;
		QStringList texts;
		QList<int> rows;
		for (const auto& index : indexes)
			if (!rows.contains (index.row ()))
				rows << index.row ();
		for (const auto& row : rows)
		{
			const auto& item = Items_ [row];
			urls << QUrl (item.URL_);
			texts << item.Title_;
		}

		auto data = new QMimeData ();
		data->setUrls (urls);
		data->setText (texts.join (";"));
		return data;
	}

	bool FavoritesModel::dropMimeData (const QMimeData *data, Qt::DropAction, int, int, const QModelIndex&)
	{
		const auto& urls = data->urls ();

		QStringList visibleTags;
		if (data->hasFormat ("x-leechcraft/tag"))
		{
			auto tm = Core::Instance ().GetProxy ()->GetTagsManager ();
			const auto& visible = tm->GetTag (data->data ("x-leechcraft/tag"));
			if (!visible.isEmpty ())
				visibleTags << visible;
		}

		auto tryAddUrl = [&visibleTags, this] (const QString& title, const QUrl& url) -> void
		{
			const auto pos = std::find_if (Items_.begin (), Items_.end (),
					[&title] (const FavoritesItem& item) { return item.Title_ == title; });
			if (pos == Items_.end ())
				addItem (title, url.toString (), visibleTags);
			else
			{
				auto tags = pos->Tags_;
				tags += visibleTags;
				setData (index (std::distance (Items_.begin (), pos), ColumnTags), tags);
			}
		};

		if (urls.size () == 1 && !data->text ().isEmpty ())
			tryAddUrl (data->text (), urls.first ());
		else if (!urls.isEmpty ())
		{
			auto texts = data->text ().split (';', QString::SkipEmptyParts);
			if (texts.size () != urls.size ())
			{
				texts.clear ();
				for (const auto& url : urls)
					texts << QFileInfo (url.path ()).fileName ();
			}

			for (const auto& pair : Util::Views::Zip (texts, urls))
				tryAddUrl (pair.first, pair.second);
		}

		return true;
	}

	void FavoritesModel::EditBookmark (const QModelIndex& source)
	{
		const auto& currentURL = source.sibling (source.row (),
				FavoritesModel::ColumnURL).data ().toString ();

		EditBookmarkDialog dia (source);
		if (dia.exec () != QDialog::Accepted)
			return;

		setData (source.sibling (source.row (), FavoritesModel::ColumnTitle), dia.GetTitle ());
		setData (source.sibling (source.row (), FavoritesModel::ColumnTags), dia.GetTags ());

		if (currentURL != dia.GetURL ())
			ChangeURL (source, dia.GetURL ());
	}

	void FavoritesModel::ChangeURL (const QModelIndex& index,
			const QString& url)
	{
		FavoritesItem item = Items_.at (index.row ());
		if (item.URL_ == url)
			return;

		removeItem (index);
		item.URL_ = url;
		Core::Instance ().GetStorageBackend ()->AddToFavorites (item);
	}

	const FavoritesModel::items_t& FavoritesModel::GetItems () const
	{
		return Items_;
	}

	namespace
	{
		struct ItemFinder
		{
			const QString& URL_;

			ItemFinder (const QString& url)
			: URL_ (url)
			{
			}

			bool operator() (const FavoritesModel::FavoritesItem& item) const
			{
				return item.URL_ == URL_;
			}
		};
	};

	void FavoritesModel::SetCheckResults (const QMap<QString, QString>& res)
	{
		CheckResults_ = res;
	}

	bool FavoritesModel::IsUrlExists (const QString& url) const
	{
		return std::any_of (Items_.begin (), Items_.end (), ItemFinder (url));
	}

	QStringList FavoritesModel::GetVisibleTags (int index) const
	{
		return Core::Instance ().GetProxy ()->GetTagsManager ()->GetTags (Items_ [index].Tags_);
	}

	FavoritesModel::FavoritesItem FavoritesModel::GetItemFromUrl (const QString& url)
	{
		for (const auto& item : Items_)
			if (item.URL_ == url)
				return item;

		return {};
	}

	void FavoritesModel::removeItem (const QModelIndex& index)
	{
		if (!index.isValid () ||
				index.row () < 0 ||
				index.row () > Items_.size ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index"
					<< index
					<< Items_.size ();
			return;
		}

		const QString url = Items_ [index.row ()].URL_;
		Core::Instance ().GetStorageBackend ()->RemoveFromFavorites (Items_ [index.row ()]);
		Core::Instance ().RemoveFromFavorites (url);
	}

	void FavoritesModel::removeItem (const QString& url)
	{
		const FavoritesItem& item = GetItemFromUrl (url);
		Core::Instance ().GetStorageBackend ()->RemoveFromFavorites (item);
		Core::Instance ().RemoveFromFavorites (url);
	}

	void FavoritesModel::handleItemAdded (const FavoritesModel::FavoritesItem& item)
	{
		beginInsertRows (QModelIndex (), rowCount (), rowCount ());
		Items_.push_back (item);
		endInsertRows ();
	}

	void FavoritesModel::handleItemUpdated (const FavoritesModel::FavoritesItem& item)
	{
		const auto pos = std::find_if (Items_.begin (), Items_.end (), ItemFinder (item.URL_));
		if (pos == Items_.end ())
		{
			qWarning () << Q_FUNC_INFO << "not found updated item";
			return;
		}

		*pos = item;

		int n = std::distance (Items_.begin (), pos);

		emit dataChanged (index (n, 0), index (n, 2));
	}

	void FavoritesModel::handleItemRemoved (const FavoritesModel::FavoritesItem& item)
	{
		const auto pos = std::find (Items_.begin (), Items_.end (), item);
		if (pos == Items_.end ())
		{
			qWarning () << Q_FUNC_INFO << "not found removed item";
			return;
		}

		int n = std::distance (Items_.begin (), pos);
		beginRemoveRows (QModelIndex (), n, n);
		Items_.erase (pos);
		endRemoveRows ();
	}

	void FavoritesModel::loadData ()
	{
		items_t items;
		Core::Instance ().GetStorageBackend ()->LoadFavorites (items);

		if (!items.size ())
			return;

		beginInsertRows (QModelIndex (), 0, items.size () - 1);
		for (items_t::iterator i = items.begin (), end = items.end (); i != end; ++i)
		{
			for (const auto& tag : QStringList { i->Tags_ })
			{
				const auto& ut = Core::Instance ().GetProxy ()->GetTagsManager ()->GetTag (tag);
				if (ut.isEmpty ())
					i->Tags_.removeAll (tag);
			}

			Items_.push_back (*i);
		}
		endInsertRows ();
	}
}
}
