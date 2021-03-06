/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelparticipantentry.h"
#include <QMenu>
#include <interfaces/core/icoreproxy.h>
#include "channelhandler.h"
#include "channelclentry.h"
#include "ircmessage.h"
#include "ircaccount.h"
#include "channelsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	ChannelParticipantEntry::ChannelParticipantEntry (const QString& nick,
			ChannelHandler *ich, IrcAccount *acc)
	: IrcParticipantEntry (nick, acc)
	, ICH_ (ich)
	{
		QMenu *infoMenu = new QMenu (tr ("Information"));
		infoMenu->addAction ("/WHOIS " + Nick_,
				this,
				SLOT (handleWhoIs ()));
		infoMenu->addAction ("/WHOWAS " + Nick_,
				this,
				SLOT (handleWhoWas ()));
		infoMenu->addAction ("/WHO " + Nick_,
				this,
				SLOT (handleWho ()));

		QMenu *ctcpMenu = new QMenu (tr ("CTCP"));
		QAction *ping = ctcpMenu->addAction ("PING");
		ping->setProperty ("ctcp_type", "ping");
		QAction *finger = ctcpMenu->addAction ("FINGER");
		finger->setProperty ("ctcp_type", "finger");
		QAction *version = ctcpMenu->addAction ("VERSION");
		version->setProperty ("ctcp_type", "version");
		QAction *userinfo = ctcpMenu->addAction ("USERINFO");
		userinfo->setProperty ("ctcp_type", "userinfo");
		QAction *clientinfo = ctcpMenu->addAction ("CLIENTINFO");
		clientinfo->setProperty ("ctcp_type", "clientinfo");
		QAction *source = ctcpMenu->addAction ("SOURCE");
		source->setProperty ("ctcp_type", "source");
		QAction *time = ctcpMenu->addAction ("TIME");
		time->setProperty ("ctcp_type", "time");

		connect (ctcpMenu,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (handleCTCPAction (QAction*)));

		Actions_.append (infoMenu->menuAction ());
		Actions_.append (ctcpMenu->menuAction ());

		ServerID_ = ICH_->GetParentID ();
	}

	ICLEntry* ChannelParticipantEntry::GetParentCLEntry () const
	{
		return ICH_->GetCLEntry ();
	}

	QString ChannelParticipantEntry::GetEntryID () const
	{
		return Account_->GetAccountName () + "/" +
				ServerID_ + "_" + Nick_;
	}

	QString ChannelParticipantEntry::GetHumanReadableID () const
	{
		return Nick_ + "_" + ICH_->GetChannelID ();
	}

	QStringList ChannelParticipantEntry::Groups () const
	{
		return QStringList (ICH_->GetChannelID ());
	}

	void ChannelParticipantEntry::SetGroups (const QStringList&)
	{
	}

	IMessage* ChannelParticipantEntry::CreateMessage (IMessage::Type,
			const QString&, const QString& body)
	{
		IrcMessage *message = new IrcMessage (IMessage::Type::ChatMessage,
				IMessage::Direction::Out,
				ServerID_,
				Nick_,
				Account_->GetClientConnection ().get ());

		message->SetBody (body);
		message->SetDateTime (QDateTime::currentDateTime ());

		return message;
	}

	QList<ChannelRole> ChannelParticipantEntry::Roles () const
	{
		return Roles_;
	}

	ChannelRole ChannelParticipantEntry::HighestRole ()
	{
		if (Roles_.isEmpty ())
			return ChannelRole::Participant;

		return Roles_.last ();
	}

	void ChannelParticipantEntry::SetRole (const ChannelRole& role)
	{
		if (!Roles_.contains (role))
		{
			Roles_ << role;
			std::sort (Roles_.begin (), Roles_.end ());
			emit permsChanged ();
		}
	}

	void ChannelParticipantEntry::SetRoles (const QList<ChannelRole>& roles)
	{
		Roles_ = roles;
		std::sort (Roles_.begin (), Roles_.end ());
		emit permsChanged ();
	}

	void ChannelParticipantEntry::RemoveRole (const ChannelRole& role)
	{
		if (Roles_.removeAll (role))
			emit permsChanged ();
	}

	void ChannelParticipantEntry::handleWhoIs ()
	{
		ICH_->handleWhoIs (Nick_);
	}

	void ChannelParticipantEntry::handleWhoWas ()
	{
		ICH_->handleWhoWas (Nick_);
	}

	void ChannelParticipantEntry::handleWho ()
	{
		ICH_->handleWho (Nick_);
	}

	void ChannelParticipantEntry::handleCTCPAction (QAction *action)
	{
		ICH_->handleCTCPRequest (QStringList () << Nick_
				<< action->property ("ctcp_type").toString ());
	}

}
}
}
