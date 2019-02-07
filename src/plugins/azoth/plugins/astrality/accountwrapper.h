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

#include <Types>
#include <Account>
#include <ContactMessenger>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iextselfinfoaccount.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	class EntryWrapper;

	class AccountWrapper : public QObject
						 , public IAccount
						 , public IExtSelfInfoAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount LeechCraft::Azoth::IExtSelfInfoAccount);

		Tp::AccountPtr A_;
		const ICoreProxy_ptr Proxy_;
		QList<EntryWrapper*> Entries_;

		QMap<QString, Tp::ContactMessengerPtr> Messengers_;
	public:
		struct Settings
		{
			bool Autodisconnect_;
		};
	private:
		Settings S_;
	public:
		AccountWrapper (Tp::AccountPtr, const ICoreProxy_ptr&, QObject*);

		// IAccount
		QObject* GetQObject ();
		QObject* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		QString GetAccountName () const;
		QString GetOurNick () const;
		void RenameAccount (const QString&);
		QByteArray GetAccountID () const;
		QList<QAction*> GetActions () const;
		void OpenConfigurationDialog ();
		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&,
				const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager() const;

		// IExtSelfInfoAccount
		QObject* GetSelfContact () const;
		QIcon GetAccountIcon () const;

		Tp::ContactMessengerPtr GetMessenger (const QString&);
		QString GetOurID () const;
		void Shutdown ();
		void RemoveThis ();
		void SetSettings (const Settings&);
	private:
		void HandleAuth (bool failure);
		EntryWrapper* CreateEntry (Tp::ContactPtr);

		void LoadSettings ();
		void SaveSettings ();
	private slots:
		void handleAccountReady (Tp::PendingOperation* = 0);

		void handleEnabled (Tp::PendingOperation*);
		void handleRemoved (Tp::PendingOperation*);
		void handleRenamed (Tp::PendingOperation*);

		void handleConnStatusChanged (Tp::ConnectionStatus);
		void handleConnectionChanged (Tp::ConnectionPtr);

		void handlePasswordFixed (Tp::PendingOperation*);
		void handleRequestedPresenceFinish (Tp::PendingOperation*);
		void handleCurrentPresence (Tp::Presence);

		void handleAccountAvatar (const Tp::Avatar&);

		void handlePresencePubRequested (Tp::Contacts);
		void handleCMStateChanged (Tp::ContactListState);
		void handleKnownContactsChanged (Tp::Contacts,
				Tp::Contacts, Tp::Channel::GroupMemberChangeDetails);

		void handleAuthRequestFinished (Tp::PendingOperation*);
		void handleAuthRequestSent (Tp::PendingOperation*);
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void accountRenamed (const QString&);
		void authorizationRequested (QObject*, const QString&);
		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemUnsubscribed (const QString&, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);
		void statusChanged (const EntryStatus&);
		void mucInvitationReceived (const QVariantMap&, const QString&, const QString&);

		void removeFinished (AccountWrapper*);

		void gotEntity (LeechCraft::Entity);
	};
}
}
}
