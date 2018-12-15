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

#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <util/sll/util.h>
#include <util/db/oral/oralfwd.h>
#include "storagebackend.h"

namespace LeechCraft
{
namespace Aggregator
{
	class SQLStorageBackend : public StorageBackend
	{
		Q_OBJECT

		Util::DefaultScopeGuard DBRemover_;
		QSqlDatabase DB_;
	public:
		struct FeedR;
		struct FeedSettingsR;
		struct ChannelR;
		struct ItemR;

		struct EnclosureR;

		struct MRSSEntryR;
		struct MRSSThumbnailR;
		struct MRSSCreditR;
		struct MRSSCommentR;
		struct MRSSPeerLinkR;
		struct MRSSSceneR;

		struct Item2TagsR;
	private:
		const Type Type_;

		Util::oral::ObjectInfo_ptr<FeedR> Feeds_;
		Util::oral::ObjectInfo_ptr<FeedSettingsR> FeedsSettings_;
		Util::oral::ObjectInfo_ptr<ChannelR> Channels_;
		Util::oral::ObjectInfo_ptr<ItemR> Items_;
		Util::oral::ObjectInfo_ptr<EnclosureR> Enclosures_;
		Util::oral::ObjectInfo_ptr<MRSSThumbnailR> MRSSThumbnails_;
		Util::oral::ObjectInfo_ptr<MRSSCreditR> MRSSCredits_;
		Util::oral::ObjectInfo_ptr<MRSSCommentR> MRSSComments_;
		Util::oral::ObjectInfo_ptr<MRSSPeerLinkR> MRSSPeerLinks_;
		Util::oral::ObjectInfo_ptr<MRSSSceneR> MRSSScenes_;
		Util::oral::ObjectInfo_ptr<MRSSEntryR> MRSSEntries_;
		Util::oral::ObjectInfo_ptr<Item2TagsR> Items2Tags_;
	public:
		SQLStorageBackend (Type, const QString&);

		void Prepare () override;

		ids_t GetFeedsIDs () const override;
		Feed GetFeed (IDType_t) const override;
		std::optional<IDType_t> FindFeed (const QString&) const override;
		std::optional<Feed::FeedSettings> GetFeedSettings (IDType_t) const override;
		void SetFeedSettings (const Feed::FeedSettings&) override;

		channels_shorts_t GetChannels (IDType_t) const override;
		Channel GetChannel (IDType_t) const override;
		std::optional<IDType_t> FindChannel (const QString& , const QString&, IDType_t) const override;
		void TrimChannel (IDType_t, int, int) override;
		std::optional<QImage> GetChannelPixmap (IDType_t) const override;
		void SetChannelPixmap (IDType_t, const std::optional<QImage>&) override;
		void SetChannelFavicon (IDType_t, const std::optional<QImage>&) override;
		void SetChannelTags (IDType_t, const QStringList&) override;
		void SetChannelDisplayTitle (IDType_t, const QString&) override;
		void SetChannelTitle (IDType_t, const QString&) override;
		void SetChannelLink (IDType_t, const QString&) override;

		items_shorts_t GetItems (IDType_t) const override;
		int GetUnreadItemsCount (IDType_t) const override;
		int GetTotalItemsCount (IDType_t) const override;
		std::optional<Item> GetItem (IDType_t) const override;
		std::optional<IDType_t> FindItem (const QString&, const QString&, IDType_t) const override;
		std::optional<IDType_t> FindItemByLink (const QString&, IDType_t) const override;
		std::optional<IDType_t> FindItemByTitle (const QString&, IDType_t) const override;
		items_container_t GetFullItems (IDType_t) const override;

		void AddFeed (const Feed&) override;
		void UpdateChannel (const ChannelShort&) override;
		void UpdateItem (const Item&) override;
		void SetItemUnread (IDType_t, bool) override;
		void AddChannel (const Channel&) override;
		void AddItem (const Item&) override;
		void RemoveItems (const QSet<IDType_t>&) override;
		void RemoveChannel (IDType_t) override;
		void RemoveFeed (IDType_t) override;
		bool UpdateFeedsStorage (int, int) override;
		bool UpdateChannelsStorage (int, int) override;
		bool UpdateItemsStorage (int, int) override;
		void ToggleChannelUnread (IDType_t, bool) override;

		QList<ITagsManager::tag_id> GetItemTags (IDType_t) override;
		void SetItemTags (IDType_t, const QList<ITagsManager::tag_id>&) override;
		QList<IDType_t> GetItemsForTag (const ITagsManager::tag_id&) override;

		IDType_t GetHighestID (const PoolType&) const override;
	private:
		void WriteEnclosures (const QList<Enclosure>&);
		void GetEnclosures (IDType_t, QList<Enclosure>&) const;
		void WriteMRSSEntries (const QList<MRSSEntry>&);
		void GetMRSSEntries (IDType_t, QList<MRSSEntry>&) const;
		IDType_t GetHighestID (const QString&, const QString&) const;
	};
}
}
