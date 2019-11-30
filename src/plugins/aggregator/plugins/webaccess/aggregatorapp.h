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

#include <QCoreApplication>
#include <Wt/WApplication.h>
#include <Wt/WModelIndex.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/aggregator/item.h>

class QThread;

namespace LC
{
namespace Aggregator
{
class IProxyObject;

namespace WebAccess
{
	class Q2WProxyModel;
	class ReadChannelsFilter;
	class ReadItemsFilter;

	class AggregatorApp : public Wt::WApplication
	{
		Q_DECLARE_TR_FUNCTIONS (AggregatorApp)

		IProxyObject *AP_;
		ICoreProxy_ptr CP_;

		QThread * const ObjsThread_;

		std::shared_ptr<Q2WProxyModel> ChannelsModel_;
		std::shared_ptr<ReadChannelsFilter> ChannelsFilter_;

		QAbstractItemModel * const SourceItemModel_;
		std::shared_ptr<Q2WProxyModel> ItemsModel_;
		std::shared_ptr<ReadItemsFilter> ItemsFilter_;

		Wt::WTableView *ItemsTable_;

		Wt::WText *ItemView_;
	public:
		enum ChannelRole
		{
			CID = Wt::ItemDataRole::User + 1,
			FID,
			UnreadCount
		};

		enum ItemRole
		{
			IID = Wt::ItemDataRole::User + 1,
			IsRead
		};

		AggregatorApp (IProxyObject*, ICoreProxy_ptr, const Wt::WEnvironment& environment);
		~AggregatorApp ();
	private:
		void HandleChannelClicked (const Wt::WModelIndex&, const Wt::WMouseEvent&);
		void HandleItemClicked (const Wt::WModelIndex&, const Wt::WMouseEvent&);

		void ShowItem (const QModelIndex&, const Item&);
		void ShowItemMenu (const QModelIndex&, const Item&, const Wt::WMouseEvent&);

		void SetupUI ();
	};
}
}
}
