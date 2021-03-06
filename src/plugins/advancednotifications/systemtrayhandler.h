/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMap>
#include <QStringList>
#include <QPixmap>
#include <QPointer>
#include <QSystemTrayIcon>
#include <interfaces/structures.h>
#include <interfaces/iactionsexporter.h>
#include "concretehandlerbase.h"
#include "eventdata.h"

class QSystemTrayIcon;

namespace LC
{
namespace AdvancedNotifications
{
	class VisualNotificationsView;

	class SystemTrayHandler : public ConcreteHandlerBase
	{
		Q_OBJECT

		QMap<QString, QSystemTrayIcon*> Category2Icon_;
		QMap<QString, QAction*> Category2Action_;
		QMap<QString, EventData> Events_;

		QMap<QSystemTrayIcon*, QList<EventData>> EventsForIcon_;
		QMap<QSystemTrayIcon*, VisualNotificationsView*> Icon2NotificationView_;

		QMap<QAction*, QList<EventData>> EventsForAction_;
		QMap<QAction*, VisualNotificationsView*> Action2NotificationView_;
	public:
		SystemTrayHandler ();
		~SystemTrayHandler ();

		NotificationMethod GetHandlerMethod () const;
		void Handle (const Entity&, const NotificationRule&);
	private:
		void PrepareSysTrayIcon (const QString&);
		void PrepareLCTrayAction (const QString&);
		void UpdateMenu (QMenu*, const QString&, const EventData&);
		void RebuildState ();
		template<typename T>
		void UpdateIcon (T iconable, const QString&);
		void UpdateSysTrayIcon (QSystemTrayIcon*);
		void UpdateTrayAction (QAction*);
	private slots:
		void handleActionTriggered ();
		void handleActionTriggered (const QString&, int);
		void dismissNotification ();
		void dismissNotification (const QString&);

		void handleTrayActivated (QSystemTrayIcon::ActivationReason);
		void handleLCAction ();
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
}
}
