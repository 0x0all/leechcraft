/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCLENTRY_H

#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QStringList>
#include <interfaces/iclentry.h>
#include <interfaces/imucentry.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class IrcAccount;
	class ChannelPublicMessage;
	class ChannelHandler;
	
	class ChannelCLEntry : public QObject
							, public ICLEntry
							, public IMUCEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry
						LeechCraft::Azoth::IMUCEntry);
		
		IrcAccount *Account_;
		QList<QObject*> AllMessages_;
		ChannelHandler *CH_;
	public:
		ChannelCLEntry (ChannelHandler*, IrcAccount*);
		
		ChannelHandler* GetChannelHandler () const;
		IrcAccount* GetIrcAccount () const;
		// ICLEntry
		QObject* GetObject ();
		QObject* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		QList<QObject*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		
		EntryStatus GetStatus (const QString& variant = QString ()) const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		bool MayChangeAffiliation (QObject*, MUCAffiliation) const;
		bool MayChangeRole (QObject*, MUCRole) const;
		MUCAffiliation GetAffiliation (QObject*) const;
		void SetAffiliation (QObject*, MUCAffiliation, const QString&);
		MUCRole GetRole (QObject*) const;
		void SetRole (QObject*, MUCRole, const QString&);

		// IMUCEntry
		MUCFeatures GetMUCFeatures () const;
		QString GetMUCSubject () const;
		void SetMUCSubject (const QString&);
		QList<QObject*> GetParticipants ();
		void Leave (const QString&);
		QString GetNick () const;
		void SetNick (const QString&);
		
		void HandleMessage (ChannelPublicMessage*);
		void HandleNewParticipants (const QList<ICLEntry*>&);
		void HandleSubjectChanged (const QString&);
	signals:
		void gotMessage (QObject*);
		void gotNewParticipants (const QList<QObject*>&);
		void mucSubjectChanged (const QString&);
		
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void avatarChanged (const QImage&);
		void rawinfoChanged (const QString&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void participantAffiliationChanged (QObject*, MUCAffiliation);
		void participantRoleChanged (QObject*, MUCRole);
		void chatPartStateChanged (const ChatPartState&, const QString&);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCLENTRY_H
