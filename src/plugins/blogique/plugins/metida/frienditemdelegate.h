/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStyledItemDelegate>

class QTreeView;
class QSortFilterProxyModel;

namespace LC
{
namespace Blogique
{
namespace Metida
{
	const int CPadding = 2;

	enum ItemColorRoles
	{
		BackgroundColor = Qt::UserRole + 1,
		ForegroundColor = Qt::UserRole,

		MaxColor
	};

	enum ItemGroupRoles
	{
		GroupId = MaxColor + 1
	};

	class FriendItemDelegate : public QStyledItemDelegate
	{
		Q_OBJECT

		bool ColoringItems_;
		QTreeView *View_;
		QSortFilterProxyModel *SortModel_;

		enum Columns
		{
			UserName
		};
	public:
		FriendItemDelegate (QSortFilterProxyModel *sortModel, QTreeView *view = 0);
		void paint (QPainter *painter, const QStyleOptionViewItem& option,
				const QModelIndex& index) const;

	public slots:
		void handleColoringItemChanged ();
	};
}
}
}
