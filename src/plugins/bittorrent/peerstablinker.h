/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPersistentModelIndex>

class QSortFilterProxyModel;

namespace Ui
{
	class TorrentTabWidget;
}

namespace LeechCraft
{
namespace Plugins
{
namespace BitTorrent
{
	class PeersTabLinker : public QObject
	{
		Q_OBJECT

		Ui::TorrentTabWidget *Ui_;
		QSortFilterProxyModel *ProxyModel_;
		QPersistentModelIndex Current_;
	public:
		PeersTabLinker (Ui::TorrentTabWidget*,
				QSortFilterProxyModel*, QObject* = 0);
	private slots:
		void handleNewRow (const QModelIndex&);
		void update ();
	};
}
}
}
