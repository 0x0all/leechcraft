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

#include "kinotify.h"
#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <util/sys/resourceloader.h>
#include <util/util.h>
#include <interfaces/entitytesthandleresult.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "kinotifywidget.h"
#include "xmlsettingsmanager.h"
#include "fswinwatcher.h"

namespace LeechCraft
{
namespace Kinotify
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("kinotify");

		Proxy_ = proxy;

		ThemeLoader_.reset (new Util::ResourceLoader ("kinotify/themes/notification"));
		ThemeLoader_->AddLocalPrefix ();
		ThemeLoader_->AddGlobalPrefix ();

		connect (ThemeLoader_.get (),
				SIGNAL (watchedDirectoriesChanged ()),
				this,
				SLOT (handleWatchedDirsChanged ()));

		SettingsDialog_.reset (new Util::XmlSettingsDialog ());
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"kinotifysettings.xml");

		SettingsDialog_->SetDataSource ("NotificatorStyle",
				ThemeLoader_->GetSubElemModel ());

		FSWinWatcher_.reset (new FSWinWatcher (Proxy_));
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Kinotify";
	}

	QString Plugin::GetName () const
	{
		return "Kinotify";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Fancy Kinetic notifications.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/plugins/kinotify/resources/images/kinotify.svg");
		return icon;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		const bool could = e.Mime_ == "x-leechcraft/notification" &&
				e.Additional_ ["Priority"].toInt () != PLog_ &&
				!e.Additional_ ["Text"].toString ().isEmpty ();
		return could ?
				EntityTestHandleResult (EntityTestHandleResult::PHigh) :
				EntityTestHandleResult ();
	}

	namespace
	{
		void OverridePixmap (KinotifyWidget *notificationWidget,
				const QVariant& notifVar, Priority prio, const ICoreProxy_ptr& proxy)
		{
			if (notifVar.canConvert<QPixmap> ())
			{
				const auto& pixmap = notifVar.value<QPixmap> ();
				if (!pixmap.isNull ())
				{
					notificationWidget->OverrideImage (pixmap);
					return;
				}
			}
			else if (notifVar.canConvert<QImage> ())
			{
				const auto& image = notifVar.value<QImage> ();
				if (!image.isNull ())
				{
					notificationWidget->OverrideImage (image);
					return;
				}
			}

			QString mi = "dialog-information";
			switch (prio)
			{
				case PWarning_:
					mi = "dialog-warning";
					break;
				case PCritical_:
					mi = "dialog-error";
				default:
					break;
			}

			const auto& icon = proxy->GetIconThemeManager ()->GetIcon (mi);
			const auto& px = icon.pixmap ({ 128, 128 });
			notificationWidget->OverrideImage (px);
		}
	}

	void Plugin::Handle (Entity e)
	{
		if (XmlSettingsManager::Instance ()->
					property ("RespectFullscreen").toBool () &&
				FSWinWatcher_->IsCurrentFS ())
			return;

		Priority prio = static_cast<Priority> (e.Additional_ ["Priority"].toInt ());
		if (prio == PLog_)
			return;

		const auto& sender = e.Additional_ ["org.LC.AdvNotifications.SenderID"].toString ();
		const auto& event = e.Additional_ ["org.LC.AdvNotifications.EventID"].toString ();;
		const auto& notifyId = sender + event;

		auto sameIdPos = notifyId.isEmpty () ?
				ActiveNotifications_.end () :
				std::find_if (ActiveNotifications_.begin (), ActiveNotifications_.end (),
						[&notifyId] (KinotifyWidget *w) { return notifyId == w->GetID (); });

		const auto& header = e.Entity_.toString ();
		const auto& text = e.Additional_ ["Text"].toString ();
		const auto sameDataPos =
				std::find_if (ActiveNotifications_.begin (), ActiveNotifications_.end (),
						[&header, &text] (KinotifyWidget *w)
							{ return w->GetTitle () == header && w->GetBody () == text; });
		if (sameDataPos != ActiveNotifications_.end () && sameIdPos == ActiveNotifications_.end ())
			return;

		const auto defaultTimeout = XmlSettingsManager::Instance ()->
				property ("MessageTimeout").toInt () * 1000;
		const auto timeout = e.Additional_.value ("NotificationTimeout", defaultTimeout).toInt ();

		auto notificationWidget = new KinotifyWidget (Proxy_, timeout);
		notificationWidget->SetID (notifyId);
		notificationWidget->SetThemeLoader (ThemeLoader_);
		notificationWidget->SetEntity (e);

		QStringList actionsNames = e.Additional_ ["NotificationActions"].toStringList ();
		if (!actionsNames.isEmpty ())
		{
			if (!e.Additional_ ["HandlingObject"].canConvert<QObject_ptr> ())
				qWarning () << Q_FUNC_INFO
						<< "value is not QObject_ptr"
						<< e.Additional_ ["HandlingObject"];
			else
			{
				QObject_ptr actionObject = e.Additional_ ["HandlingObject"].value<QObject_ptr> ();
				notificationWidget->SetActions (actionsNames, actionObject);
			}
		}

		connect (notificationWidget,
				SIGNAL (checkNotificationQueue ()),
				this,
				SLOT (pushNotification ()));
		connect (notificationWidget,
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));

		notificationWidget->SetContent (header, text, QString ());

		OverridePixmap (notificationWidget, e.Additional_ ["NotificationPixmap"], prio, Proxy_);

		if (!ActiveNotifications_.size ())
			notificationWidget->PrepareNotification ();

		if (sameIdPos == ActiveNotifications_.end ())
			ActiveNotifications_ << notificationWidget;
		else if (sameIdPos == ActiveNotifications_.begin ())
		{
			auto oldNotify = *sameIdPos;
			std::advance (sameIdPos, 1);
			ActiveNotifications_.insert (sameIdPos, notificationWidget);
			oldNotify->closeNotificationWidget ();
		}
		else
		{
			(*sameIdPos)->deleteLater ();
			auto newPos = ActiveNotifications_.erase (sameIdPos);
			ActiveNotifications_.insert (newPos, notificationWidget);
		}
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::pushNotification ()
	{
		if (!ActiveNotifications_.size ())
			return;

		ActiveNotifications_.removeFirst ();
		if (ActiveNotifications_.size ())
			ActiveNotifications_.first ()->PrepareNotification ();
	}

	void Plugin::handleWatchedDirsChanged ()
	{
		KinotifyWidget::ClearThemeCache ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_kinotify, LeechCraft::Kinotify::Plugin);
