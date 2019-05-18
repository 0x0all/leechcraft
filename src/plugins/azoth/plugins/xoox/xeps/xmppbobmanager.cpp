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

#include "xmppbobmanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include "xmppbobiq.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NSBob = "urn:xmpp:bob";

	XMPPBobManager::XMPPBobManager (unsigned int cacheSizeKb)
	: BobCache_ (cacheSizeKb)
	{
	}

	bool XMPPBobManager::handleStanza (const QDomElement& stanza)
	{
		XMPPBobIq bobIq;
		bool requestReceived = false;

		const auto& tagName = stanza.tagName ();
		if (tagName == "iq")
		{
			if (XMPPBobIq::IsBobIq (stanza))
			{
				requestReceived = true;
				bobIq.parse (stanza);
			}
			else if (XMPPBobIq::IsBobIq (stanza.firstChildElement ()))
				bobIq.parse (stanza.firstChildElement ());
		}
		else if ((tagName == "message" || tagName == "presence") &&
				XMPPBobIq::IsBobIq (stanza))
			bobIq.parse (stanza);
		else
			return false;

		BobCache_.insert (qMakePair (bobIq.GetCid (), bobIq.from ()),
				new QByteArray (bobIq.GetData ()),
				bobIq.GetData ().size () / 1024);

		if (requestReceived)
			emit bobReceived (bobIq);

		return requestReceived;
	}

	QStringList XMPPBobManager::discoveryFeatures () const
	{
		return QStringList (NSBob);
	}

	QString XMPPBobManager::RequestBob (const QString& jid, const QString& cid)
	{
		XMPPBobIq request;
		request.setType (QXmppIq::Get);
		request.setTo (jid);
		request.SetCid (cid);
		if (client ()->sendPacket (request))
			return request.id ();
		else
			return QString ();
	}

	QByteArray XMPPBobManager::Take (const QString& jid, const QString& cid)
	{
		QPair<QString, QString> key (cid, jid);
		if (BobCache_.contains (key))
			return *BobCache_ [key];
		return QByteArray ();
	}
}
}
}