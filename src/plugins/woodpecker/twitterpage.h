/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Slava Barinov <rayslava@gmail.com>
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

#include <QWidget>
#include <QScrollBar>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <leechcraft/interfaces/core/ientitymanager.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/structures.h>
#include "twitterinterface.h"
#include "twitdelegate.h"
#include "ui_twitterpage.h"

namespace LeechCraft
{
namespace Woodpecker
{
	class TwitterPage : public QWidget
						, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		const TabClassInfo TC_;
		QObject *const ParentPlugin_;

		QToolBar *Toolbar_;
		QMenu *DoctypeMenu_;
		QMenu *RecentFilesMenu_;
		IEntityManager *EntityManager_;
		QString Filename_;
		bool Modified_;
		QMap<QString, QList<QAction*>> WindowMenus_;
		QHash<QString, QString> Extension2Lang_;
		
		QtMsgHandler DefaultMsgHandler_;
		QObject *WrappedObject_;
		bool TemporaryDocument_;
		
		bool UpdateReady_;			/**< The flag is checked by timer for UI update */
		QTimer *UiUpdateTimer_;	/**< Timer checks UpdateReady_ and updates the UI */
		TwitDelegate *Delegate_;
		Ui::TwitterPage Ui_;
		TwitterInterface *Interface_;
		QTimer *TwitterTimer_;		/**< This timer sends network requests to get new twits */
		QSettings *Settings_;
		QList<std::shared_ptr<Tweet>> ScreenTwits_;

		QAction *ActionRetwit_;
		QAction *ActionReply_;
		QAction *ActionSPAM_;
		QAction *ActionOpenWeb_;
		
	private slots:
		void on_TwitList__customContextMenuRequested (const QPoint&);
		void updateTweetList ();
		
	public:
		explicit TwitterPage (const TabClassInfo&, QObject*);
		~TwitterPage();
		
		void Remove ();
		QToolBar *GetToolBar () const;
		QObject *ParentMultiTabs ();
		QList<QAction*> GetTabBarContextMenuActions () const;
		QMap<QString, QList<QAction*>> GetWindowMenus () const;
		TabClassInfo GetTabClassInfo () const;
		
	public slots:
		void tryToLogin ();
		void requestUserTimeline (const QString& username);
		void updateScreenTwits (QList<std::shared_ptr<Tweet>> twits);
		void recvdAuth (const QString& token, const QString& tokenSecret);
		void twit ();
		void retwit ();
		void reply (QListWidgetItem *index = nullptr);
		void reportSpam ();
		void sendReply ();
		void webOpen ();
		void scrolledDown (int sliderPos);
		void setUpdateReady ();
		
	signals:
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void couldHandle (const LeechCraft::Entity&, bool*);
		void delegateEntity (const LeechCraft::Entity&,
							 int*, QObject**);
		void gotEntity (const LeechCraft::Entity&);
	};
}
}

