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

#include "callshandler.h"
#include <QHostAddress>
#include <QXmppCallManager.h>
#include <util/xpc/util.h>
#include "accountsettingsholder.h"
#include "clientconnection.h"
#include "core.h"
#include "glooxaccount.h"

#ifdef ENABLE_MEDIACALLS
#include "mediacall.h"
#endif

namespace LC::Azoth::Xoox
{
	CallsHandler::CallsHandler (QXmppCallManager& callMgr, ClientConnection& conn, GlooxAccount& acc, QObject *parent)
	: QObject { parent }
	, Mgr_ { callMgr }
	, Conn_ { conn }
	, Acc_ { acc }
	{
#ifdef ENABLE_MEDIACALLS
		connect (&Mgr_,
				&QXmppCallManager::callReceived,
				this,
				[this] (QXmppCall *call) { emit Acc_.called (new MediaCall (&Acc_, call)); });

		auto settings = acc.GetSettings ();
		auto updateSettings = [&callMgr, settings]
		{
			callMgr.setStunServer (QHostAddress (settings->GetStunHost ()), settings->GetStunPort ());
			callMgr.setTurnServer (QHostAddress (settings->GetTurnHost ()), settings->GetTurnPort ());
			callMgr.setTurnUser (settings->GetTurnUser ());
			callMgr.setTurnPassword (settings->GetTurnPass ());
		};
		connect (settings,
				&AccountSettingsHolder::stunSettingsChanged,
				this,
				updateSettings);
		connect (settings,
				&AccountSettingsHolder::turnSettingsChanged,
				this,
				updateSettings);
		updateSettings ();
#else
		Q_UNUSED (Acc_)
		Q_UNUSED (Conn_)
#endif
	}

	QObject* CallsHandler::Call (const QString& id, const QString& variant)
	{
#ifdef ENABLE_MEDIACALLS
		if (id == qobject_cast<ICLEntry*> (Acc_.GetSelfContact ())->GetEntryID ())
		{
			Core::Instance ().SendEntity (Util::MakeNotification ("LeechCraft",
					tr ("Why would you call yourself?"),
					Priority::Warning));
			return nullptr;
		}

		auto target = GlooxCLEntry::JIDFromID (&Acc_, id);

		auto resultingVar = variant;
		if (resultingVar.isEmpty ())
		{
			const auto entryObj = Conn_.GetCLEntry (target, QString ());
			if (const auto entry = qobject_cast<GlooxCLEntry*> (entryObj))
				resultingVar = entry->Variants ().value (0);
			else
				qWarning () << Q_FUNC_INFO
						<< "null entry for"
						<< target;
		}
		if (!resultingVar.isEmpty ())
			target += '/' + resultingVar;

		const auto call = new MediaCall (&Acc_, Mgr_.call (target));
		emit Acc_.called (call);
		return call;
#else
		Q_UNUSED (id)
		Q_UNUSED (variant)
		return nullptr;
#endif
	}
}
