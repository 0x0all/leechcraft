/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>
#include <QImage>
#include <interfaces/media/audiostructs.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iadvancedclentry.h>
#include <interfaces/azoth/ihavecontacttune.h>
#include <interfaces/azoth/ihaveavatars.h>
#include "mrimaccount.h"
#include "proto/contactinfo.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
	class MRIMAccount;
	class MRIMMessage;
	class SelfAvatarFetcher;

	class MRIMBuddy final : public QObject
						  , public ICLEntry
						  , public IHaveAvatars
						  , public IHaveContactTune
						  , public IAdvancedCLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry
				LC::Azoth::IHaveAvatars
				LC::Azoth::IHaveContactTune
				LC::Azoth::IAdvancedCLEntry)

		MRIMAccount *A_;
		Proto::ContactInfo Info_;
		QString Group_;

		EntryStatus Status_;
		QList<MRIMMessage*> AllMessages_;
		bool IsAuthorized_ = true;
		bool GaveSubscription_ = true;

		QVariantMap ClientInfo_;

		QHash<quint32, QString> SentSMS_;

		QAction *UpdateNumber_;
		QAction *SendSMS_;

		SelfAvatarFetcher * const AvatarFetcher_;

		Media::AudioInfo TuneInfo_;
	public:
		MRIMBuddy (const Proto::ContactInfo&, MRIMAccount*);

		void HandleMessage (MRIMMessage*);
		void HandleAttention (const QString&);
		void HandleTune (const QString&);
		void HandleCPS (ChatPartState);
		void SetGroup (const QString&);

		/** @brief Sets whether this buddy is authorized by us.
		 *
		 * Toggles whether this buddy can see our presence.
		 *
		 * @sa IsAuthorized()
		 */
		void SetAuthorized (bool);
		/** @brief Whether this buddy is authorized by us.
		 *
		 * Returns true if we allowed this buddy to subscribe to our
		 * presence, false otherwise.
		 *
		 * @sa SetAuthorized()
		 */
		bool IsAuthorized () const;

		void SetGaveSubscription (bool);
		bool GaveSubscription () const;

		Proto::ContactInfo GetInfo () const;
		void UpdateInfo (const Proto::ContactInfo&);

		void HandleWPInfo (const QMap<QString, QString>&);

		qint64 GetID () const;
		void UpdateID (qint64);

		// ICLEntry
		QObject* GetQObject ();
		MRIMAccount* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString& name);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList& groups);
		QStringList Variants () const;
		IMessage* CreateMessage (IMessage::Type, const QString&, const QString&);
		QList<IMessage*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		void ShowInfo ();
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();
		void ChatTabClosed ();

		// IHaveContactTune
		Media::AudioInfo GetUserTune (const QString&) const;

		// IAdvancedCLEntry
		AdvancedFeatures GetAdvancedFeatures () const;
		void DrawAttention (const QString&, const QString&);

		// IHaveAvatars
		QFuture<QImage> RefreshAvatar (Size);
		bool HasAvatar () const;
		bool SupportsSize (Size) const;
	private:
		void UpdateClientVersion ();
	private slots:
		void handleUpdateNumber ();
		void handleSendSMS ();

		void handleSMSDelivered (quint32);
		void handleSMSServUnavail (quint32);
		void handleSMSBadParms (quint32);
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();

		void attentionDrawn (const QString&, const QString&);
		void moodChanged (const QString&);
		void activityChanged (const QString&);
		void tuneChanged (const QString&);
		void locationChanged (const QString&);

		void avatarChanged (QObject*);
	};
}
}
}
