/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_ZHEET_MSNACCOUNT_H
#define PLUGINS_AZOTH_PLUGINS_ZHEET_MSNACCOUNT_H
#include <QObject>
#include <interfaces/iaccount.h>
#include <msn/passport.h>

namespace MSN
{
	class NotificationServerConnection;
	class Buddy;
}

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	class MSNProtocol;
	class Callbacks;
	class MSNAccountConfigWidget;
	class MSNBuddyEntry;

	class MSNAccount : public QObject
					 , public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount);

		MSNProtocol *Proto_;

		QString Name_;
		MSN::Passport Passport_;
		QString Server_;
		int Port_;

		Callbacks *CB_;
		MSN::NotificationServerConnection *Conn_;

		EntryStatus PendingStatus_;
		bool Connecting_;
		EntryStatus CurrentStatus_;

		QHash<QString, MSNBuddyEntry*> Entries_;
	public:
		MSNAccount (const QString&, MSNProtocol* = 0);
		void Init ();

		QByteArray Serialize () const;
		static MSNAccount* Deserialize (const QByteArray&, MSNProtocol*);

		void FillConfig (MSNAccountConfigWidget*);

		MSN::NotificationServerConnection* GetNSConnection ();

		// IAccount
		QObject* GetObject ();
		QObject* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		QString GetAccountName () const;
		QString GetOurNick () const;
		void RenameAccount (const QString& name);
		QByteArray GetAccountID () const;
		QList<QAction*> GetActions () const;
		void QueryInfo (const QString&);
		void OpenConfigurationDialog ();
		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Synchronize ();
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&, const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager () const;
	private slots:
		void handleConnected ();
		void handleWeChangedState (State);
		void handleGotBuddies (const QList<MSN::Buddy*>&);
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void authorizationRequested (QObject*, const QString&);
		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemUnsubscribed (const QString&, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);
		void statusChanged (const EntryStatus&);
		void mucInvitationReceived (const QVariantMap&,
				const QString&, const QString&);

		void accountSettingsChanged ();
	};
}
}
}

#endif
