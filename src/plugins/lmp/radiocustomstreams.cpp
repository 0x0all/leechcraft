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

#include "radiocustomstreams.h"
#include <QStandardItemModel>
#include <QtDebug>
#include <interfaces/media/iradiostation.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/core/iiconthememanager.h>
#include "radiocustomstation.h"
#include "xmlsettingsmanager.h"
#include "core.h"

typedef QList<QPair<QString, QUrl>> CustomStationsList_t;
Q_DECLARE_METATYPE (CustomStationsList_t);

namespace LC
{
namespace LMP
{
	namespace
	{
		enum CustomRole
		{
			UrlRole = Media::RadioItemRole::MaxRadioRole + 1
		};
	}

	RadioCustomStreams::RadioCustomStreams (QObject *parent)
	: QObject { parent }
	, Model_ { new QStandardItemModel { this } }
	, Root_ { new QStandardItem { tr ("Custom streams") } }
	{
		Root_->setIcon (Core::Instance ().GetProxy ()->
					GetIconThemeManager ()->GetIcon ("favorites"));
		Root_->setData (Media::RadioType::CustomAddableStreams, Media::RadioItemRole::ItemType);
		Root_->setData ("org.LeechCraft.LMP.Custom", Media::RadioItemRole::RadioID);
		Root_->setEditable (false);
		Model_->appendRow (Root_);

		LoadSettings ();
	}

	QList<QAbstractItemModel*> RadioCustomStreams::GetRadioListItems () const
	{
		return { Model_ };
	}

	Media::IRadioStation_ptr RadioCustomStreams::GetRadioStation (const QModelIndex& item, const QString&)
	{
		QList<QUrl> urls;
		if (item == Root_->index ())
			urls = GetAllUrls ();
		else
			urls << item.data (CustomRole::UrlRole).toUrl ();
		return std::make_shared<RadioCustomStation> (urls, this);
	}

	void RadioCustomStreams::RefreshItems (const QList<QModelIndex>&)
	{
	}

	void RadioCustomStreams::Add (const QUrl& url, const QString& name)
	{
		if (GetAllUrls ().contains (url))
			return;

		CreateItem (url, name);

		SaveSettings ();
	}

	void RadioCustomStreams::Remove (const QModelIndex& index)
	{
		for (auto i = 0; i < Root_->rowCount (); ++i)
			if (Root_->child (i)->index () == index)
			{
				Root_->removeRow (i);
				SaveSettings ();
				break;
			}
	}

	void RadioCustomStreams::CreateItem (const QUrl& url, const QString& name)
	{
		const auto& urlStr = url.toString ();
		auto item = new QStandardItem (name.isEmpty () ? urlStr : name);
		item->setEditable (false);

		item->setToolTip (urlStr);
		item->setData (url, CustomRole::UrlRole);
		item->setData (Media::RadioType::SingleTrack, Media::RadioItemRole::ItemType);

		Media::AudioInfo info;
		info.Length_ = 0;
		info.Other_ ["URL"] = url;
		item->setData (QVariant::fromValue (QList<Media::AudioInfo> { info }),
				Media::RadioItemRole::TracksInfos);

		Root_->appendRow (item);
	}

	QList<QUrl> RadioCustomStreams::GetAllUrls () const
	{
		QList<QUrl> result;
		for (auto i = 0; i < Root_->rowCount (); ++i)
			result << Root_->child (i)->data (CustomRole::UrlRole).toUrl ();
		return result;
	}

	void RadioCustomStreams::LoadSettings ()
	{
		const auto& pairs = XmlSettingsManager::Instance ()
				.property ("CustomRadioUrls").value<CustomStationsList_t> ();
		for (const auto& pair : pairs)
			CreateItem (pair.second, pair.first);
	}

	void RadioCustomStreams::SaveSettings () const
	{
		CustomStationsList_t list;
		for (auto i = 0; i < Root_->rowCount (); ++i)
		{
			const auto item = Root_->child (i);
			list.append ({ item->text (), item->data (CustomRole::UrlRole).toUrl () });
		}
		XmlSettingsManager::Instance ()
				.setProperty ("CustomRadioUrls", QVariant::fromValue (list));
	}
}
}
