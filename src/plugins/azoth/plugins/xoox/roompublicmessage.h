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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMPUBLICMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMPUBLICMESSAGE_H
#include <QObject>
#include <QPointer>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/irichtextmessage.h>
#include "roomparticipantentry.h"

class QXmppMessage;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class RoomCLEntry;
	class RoomParticipantEntry;

	class RoomPublicMessage : public QObject
							, public IMessage
							, public IRichTextMessage
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMessage
				LeechCraft::Azoth::IRichTextMessage)

		QPointer<RoomCLEntry> ParentEntry_;
		RoomParticipantEntry_ptr ParticipantEntry_;
		QString Message_;
		QDateTime Datetime_;
		Direction Direction_;
		QString FromJID_;
		QString FromVariant_;
		MessageType Type_;
		MessageSubType SubType_;

		QString XHTML_;
	public:
		RoomPublicMessage (const QString&, RoomCLEntry*);
		RoomPublicMessage (const QString&, Direction,
				RoomCLEntry*,
				MessageType,
				MessageSubType,
				RoomParticipantEntry_ptr = RoomParticipantEntry_ptr ());
		RoomPublicMessage (const QXmppMessage&, RoomCLEntry*,
				RoomParticipantEntry_ptr = RoomParticipantEntry_ptr ());

		QObject* GetQObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		MessageType GetMessageType () const;
		MessageSubType GetMessageSubType () const;

		QObject* OtherPart () const;
		QObject* ParentCLEntry () const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString&);
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime&);

		QString GetRichBody () const;
		void SetRichBody (const QString&);
	};
}
}
}

#endif
