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

#include "serverinfostorage.h"
#include <algorithm>
#include <QTimer>
#include <QXmppDiscoveryManager.h>
#include "clientconnection.h"
#include "accountsettingsholder.h"
#include "discomanagerwrapper.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	ServerInfoStorage::ServerInfoStorage (ClientConnection *conn, AccountSettingsHolder *settings)
	: QObject (conn)
	, Conn_ (conn)
	, Settings_ (settings)
	{
		connect (Conn_->GetClient (),
				SIGNAL (connected ()),
				this,
				SLOT (handleConnected ()));
	}

	bool ServerInfoStorage::HasServerFeatures () const
	{
		return !ServerFeatures_.isEmpty ();
	}

	QStringList ServerInfoStorage::GetServerFeatures () const
	{
		return ServerFeatures_;
	}

	bool ServerInfoStorage::HasSelfFeatures () const
	{
		return !SelfJIDFeatures_.isEmpty ();
	}

	QStringList ServerInfoStorage::GetSelfFeatures () const
	{
		return SelfJIDFeatures_;
	}

	QString ServerInfoStorage::GetBytestreamsProxy () const
	{
		return BytestreamsProxy_;
	}

	void ServerInfoStorage::HandleItems (const QXmppDiscoveryIq& iq)
	{
		for (const auto& item : iq.items ())
			Conn_->GetDiscoManagerWrapper ()->RequestInfo (item.jid (),
					[this] (const QXmppDiscoveryIq& iq) { HandleItemInfo (iq); },
					false,
					item.node ());
	}

	void ServerInfoStorage::HandleItemInfo (const QXmppDiscoveryIq& iq)
	{
		auto hasIdentity = [&iq] (const QString& cat, const QString& type) -> bool
		{
			const auto& ids = iq.identities ();
			return std::any_of (ids.begin (), ids.end (),
					[&cat, &type] (const auto& id) { return id.category () == cat && id.type () == type; });
		};
		if (hasIdentity ("proxy", "bytestreams"))
		{
			BytestreamsProxy_ = iq.from ();
			emit bytestreamsProxyChanged (BytestreamsProxy_);
		}
	}

	void ServerInfoStorage::handleConnected ()
	{
		if (Settings_->GetJID () == PreviousJID_)
			return;

		ServerFeatures_.clear ();

		BytestreamsProxy_.clear ();
		emit bytestreamsProxyChanged (QString ());

		PreviousJID_ = Settings_->GetJID ();
		Server_ = PreviousJID_.mid (PreviousJID_.indexOf ('@') + 1);
		if (Server_.isEmpty ())
			return;

		Conn_->GetDiscoManagerWrapper ()->RequestInfo (Server_,
				[this] (const QXmppDiscoveryIq& iq) { ServerFeatures_ = iq.features (); },
				false);
		Conn_->GetDiscoManagerWrapper ()->RequestItems (Server_,
				[this] (const QXmppDiscoveryIq& iq) { HandleItems (iq); },
				false);
		Conn_->GetDiscoManagerWrapper ()->RequestInfo (Settings_->GetJID (),
				[this] (const QXmppDiscoveryIq& iq) { SelfJIDFeatures_ = iq.features (); },
				false);
	}
}
}
}
