/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "ircparticipantentry.h"
#include <QAction>
#include "ircmessage.h"
#include "ircaccount.h"
#include "clientconnection.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	IrcParticipantEntry::IrcParticipantEntry (const QString& nick, IrcAccount *account)
	: EntryBase (account)
	, Nick_ (nick)
	, IsPrivateChat_ (false)
	{
		QAction *closePrivate = new QAction ("Close chat", this);

		connect (closePrivate,
				SIGNAL (triggered ()),
				this,
				SLOT (handleClosePrivate ()));

		Actions_ << closePrivate;
	}

	IAccount* IrcParticipantEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features IrcParticipantEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType IrcParticipantEntry::GetEntryType () const
	{
		return EntryType::PrivateChat;
	}

	QString IrcParticipantEntry::GetEntryName () const
	{
		return Nick_;
	}

	void IrcParticipantEntry::SetEntryName (const QString& name)
	{
		Nick_ = name;

		for (const auto message : AllMessages_)
		{
			const auto msg = dynamic_cast<IrcMessage*> (message);
			if (!msg)
			{
				qWarning () << Q_FUNC_INFO
						<< "is not an object of IrcMessage"
						<< message;
				continue;
			}

			msg->SetOtherVariant (name);
		}
	}

	QStringList IrcParticipantEntry::Variants () const
	{
		return QStringList (QString ());
	}

	QString IrcParticipantEntry::GetUserName () const
	{
		return UserName_;
	}

	void IrcParticipantEntry::SetUserName (const QString& user)
	{
		UserName_ = user;
	}

	QString IrcParticipantEntry::GetHostName () const
	{
		return HostName_;
	}

	void IrcParticipantEntry::SetHostName (const QString& host)
	{
		HostName_ = host;
	}

	QString IrcParticipantEntry::GetRealName () const
	{
		return RealName_;
	}

	void IrcParticipantEntry::SetRealName (const QString& realName)
	{
		RealName_ = realName;
	}

	QString IrcParticipantEntry::GetServerID () const
	{
		return ServerID_;
	}

	bool IrcParticipantEntry::IsPrivateChat () const
	{
		return IsPrivateChat_;
	}

	void IrcParticipantEntry::SetPrivateChat (bool isPrivate)
	{
		IsPrivateChat_ = isPrivate;
	}

	void IrcParticipantEntry::handleClosePrivate ()
	{
		IsPrivateChat_ = false;
		Account_->GetClientConnection ()->ClosePrivateChat (ServerID_, Nick_);
	}

}
}
}
