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

#include "adhoccommandmanager.h"
#include <QXmppDiscoveryManager.h>
#include "clientconnection.h"
#include "util.h"
#include "clientconnectionerrormgr.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsAdHoc = "http://jabber.org/protocol/commands";

	QString AdHocCommandManager::GetAdHocFeature ()
	{
		return NsAdHoc;
	}

	AdHocCommandManager::AdHocCommandManager (ClientConnection *conn)
	: ClientConn_ { conn }
	{
		connect (ClientConn_->GetQXmppDiscoveryManager (),
				SIGNAL (itemsReceived (const QXmppDiscoveryIq&)),
				this,
				SLOT (handleItemsReceived (const QXmppDiscoveryIq&)));
	}

	QString AdHocCommandManager::QueryCommands (const QString& jid)
	{
		const auto& id = ClientConn_->GetQXmppDiscoveryManager ()->requestItems (jid, NsAdHoc);
		RegisterErrorHandler (id);
		return id;
	}

	QString AdHocCommandManager::ExecuteCommand (const QString& jid,
			const AdHocCommand& cmd)
	{
		QXmppElement command;
		command.setTagName ("command");
		command.setAttribute ("xmlns", NsAdHoc);
		command.setAttribute ("node", cmd.GetNode ());
		command.setAttribute ("action", "execute");

		QXmppIq iq { QXmppIq::Set };
		iq.setTo (jid);
		iq.setExtensions ({ command });

		const auto& id = iq.id ();
		PendingCommands_ << id;
		client ()->sendPacket (iq);
		RegisterErrorHandler (id);
		return id;
	}

	QString AdHocCommandManager::ProceedExecuting (const QString& jid,
			const AdHocResult& state, const QString& action)
	{
		QXmppElement command;
		command.setTagName ("command");
		command.setAttribute ("xmlns", NsAdHoc);
		command.setAttribute ("node", state.GetNode ());
		command.setAttribute ("sessionid", state.GetSessionID ());
		command.setAttribute ("action", action);

		QDomDocument formElem;
		{
			QByteArray ba;
			QXmlStreamWriter w { &ba };
			auto form = state.GetDataForm ();
			form.setType (QXmppDataForm::Submit);
			form.toXml (&w);
			if (!formElem.setContent (ba))
				qWarning () << Q_FUNC_INFO
						<< "unable to parse XML that was just serialized"
						<< ba;
		}
		command.appendChild (formElem.documentElement ());

		QXmppIq iq { QXmppIq::Set };
		iq.setTo (jid);
		iq.setExtensions ({ command });

		const auto& id = iq.id ();
		PendingCommands_ << id;
		client ()->sendPacket (iq);
		RegisterErrorHandler (id);
		return id;
	}

	QStringList AdHocCommandManager::discoveryFeatures () const
	{
		return { NsAdHoc };
	}

	namespace
	{
		bool ParseActions (AdHocResult& result, const QDomElement& actionsElem)
		{
			if (actionsElem.isNull ())
				return false;

			QStringList actionsList;

			const auto& def = actionsElem.attribute ("execute", "execute");

			auto actionElem = actionsElem.firstChildElement ();
			while (!actionElem.isNull ())
			{
				const auto& name = actionElem.tagName ();
				if (name != def)
					actionsList << name;

				actionElem = actionElem.nextSiblingElement ();
			}

			actionsList.prepend (def);

			result.SetActions (actionsList);

			return true;
		}

		void ParseDataForm (AdHocResult& result, QDomElement xForm)
		{
			QXmppDataForm form;

			if (!xForm.hasAttribute ("type"))
			{
				qWarning () << Q_FUNC_INFO
						<< "kludge for unknown form types";
				xForm.setAttribute ("type", "form");
			}

			form.parse (xForm);
			result.SetDataForm (form);
		}

		void ParseNotes (AdHocResult& result, const QDomElement& command)
		{
			auto noteElem = command.firstChildElement ("note");
			while (!noteElem.isNull ())
			{
				result.AddNote ({ noteElem });
				noteElem = noteElem.nextSiblingElement ("note");
			}
		}
	}

	bool AdHocCommandManager::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "iq" ||
				!PendingCommands_.contains (elem.attribute ("id")))
			return false;

		PendingCommands_.remove (elem.attribute ("id"));

		const auto& command = elem.firstChildElement ("command");
		if (command.namespaceURI () != NsAdHoc)
			return false;

		AdHocResult result;
		result.SetSessionID (command.attribute ("sessionid"));
		result.SetNode (command.attribute ("node"));

		if (!ParseActions (result, command.firstChildElement ("actions")) &&
				command.attribute ("status") == "executing")
			result.SetActions ({ "execute" });

		if (command.firstChildElement ("x").namespaceURI () == "jabber:x:data")
			ParseDataForm (result, command.firstChildElement ("x"));

		ParseNotes (result, command);

		emit gotResult (elem.attribute ("from"), result);

		return true;
	}

	void AdHocCommandManager::RegisterErrorHandler (const QString& id)
	{
		ClientConn_->GetErrorManager ()->SetErrorHandler (id,
				[this] (const QXmppIq& iq)
				{
					HandleError (iq);
					return true;
				});
	}

	void AdHocCommandManager::HandleError (const QXmppIq& iq)
	{
		const auto& err = iq.error ();
		auto errStr = ClientConn_->GetErrorManager ()->
				HandleErrorCondition (err.condition ());
		if (!err.text ().isEmpty ())
			errStr += " " + tr ("Original message: %1.")
					.arg (err.text ());
		emit gotError (errStr, iq.id ());
	}

	void AdHocCommandManager::handleItemsReceived (const QXmppDiscoveryIq& iq)
	{
		if (iq.queryNode () != NsAdHoc)
			return;

		QList<AdHocCommand> commands;
		for (const auto& item : iq.items ())
			commands.append ({ item.name (), item.node () });

		emit gotCommands (iq.from (), commands);
	}
}
}
}
