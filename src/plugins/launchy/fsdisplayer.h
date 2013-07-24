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

#pragma once

#include <memory>
#include <QObject>
#include <QHash>
#include <QDeclarativeView>
#include <interfaces/core/icoreproxy.h>
#include <util/xdg/xdgfwd.h>

class QStandardItem;
class QStandardItemModel;
class QDeclarativeView;

namespace LeechCraft
{
struct Entity;

namespace Launchy
{
	class ItemIconsProvider;
	class ItemsSortFilterProxyModel;
	class FavoritesManager;

	class FSDisplayer : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		Util::XDG::ItemsFinder *Finder_;
		FavoritesManager *FavManager_;

		QStandardItemModel *CatsModel_;
		QStandardItemModel *ItemsModel_;
		ItemsSortFilterProxyModel *ItemsProxyModel_;

		QDeclarativeView *View_;
		ItemIconsProvider *IconsProvider_;

		typedef std::function<void ()> Executor_f;
		struct ItemInfo
		{
			Executor_f Exec_;
			QString PermanentID_;
		};
		QHash<QString, ItemInfo> ItemInfos_;
	public:
		FSDisplayer (ICoreProxy_ptr,
				Util::XDG::ItemsFinder *finder, FavoritesManager*, QObject* = 0);
		~FSDisplayer ();
	private:
		void MakeStdCategories ();
		void MakeStdItems ();
		void MakeCategories (const QStringList&);
		void MakeItems (const QList<QList<Util::XDG::Item_ptr>>&);
		QStandardItem* FindItem (const QString&) const;
	private slots:
		void handleFinderUpdated ();
		void handleCategorySelected (int);
		void handleExecRequested (const QString&);
		void handleItemBookmark (const QString&);
		void handleViewStatus (QDeclarativeView::Status);
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
