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

#include "importmanager.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/isupportimport.h"
#include "interfaces/azoth/ihistoryplugin.h"
#include "core.h"
#include "accounthandlerchooserdialog.h"

namespace LeechCraft
{
namespace Azoth
{
	ImportManager::ImportManager (QObject *parent)
	: QObject (parent)
	{
	}

	void ImportManager::HandleAccountImport (Entity e)
	{
		const auto& map = e.Additional_ ["AccountData"].toMap ();
		const auto& protoId = map ["Protocol"].toString ();
		if (protoId.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty protocol id"
					<< map;
			return;
		}

		for (const auto proto : Core::Instance ().GetProtocols ())
		{
			const auto isi = qobject_cast<ISupportImport*> (proto->GetQObject ());
			if (!isi || isi->GetImportProtocolID () != protoId)
				continue;

			isi->ImportAccount (map);
			break;
		}
	}

	namespace
	{
		IMessage::Direction GetDirection (const QByteArray& dirStr)
		{
			if (dirStr == "out")
				return IMessage::Direction::Out;
			else if (dirStr == "in")
				return IMessage::Direction::In;

			qWarning () << Q_FUNC_INFO
					<< "unknown direction"
					<< dirStr;
			return IMessage::Direction::In;
		}

		IMessage::Type GetMessageType (const QByteArray& typeStr)
		{
			if (typeStr == "chat")
				return IMessage::Type::ChatMessage;
			else if (typeStr == "muc")
				return IMessage::Type::MUCMessage;
			else if (typeStr == "event")
				return IMessage::Type::EventMessage;

			qWarning () << Q_FUNC_INFO
					<< "unknown type"
					<< typeStr;
			return IMessage::Type::ChatMessage;
		}

		IMessage::EscapePolicy GetEscapePolicy (const QByteArray& polStr)
		{
			if (polStr.isEmpty ())
				return IMessage::EscapePolicy::Escape;
			else if (polStr == "escape")
				return IMessage::EscapePolicy::Escape;
			else if (polStr == "noEscape")
				return IMessage::EscapePolicy::NoEscape;

			qWarning () << Q_FUNC_INFO
					<< "unknown escape policy"
					<< polStr;
			return IMessage::EscapePolicy::Escape;
		}
	}

	void ImportManager::HandleHistoryImport (Entity e)
	{
		qDebug () << Q_FUNC_INFO;
		const auto& histories = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableTo<IHistoryPlugin*> ();
		if (histories.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no history plugin is present, aborting";
			return;
		}

		const auto acc = GetAccountID (e);
		if (!acc)
			return;

		const auto isi = qobject_cast<ISupportImport*> (acc->GetParentProtocol ());

		QHash<QString, QString> entryIDcache;

		QVariantList history;
		for (const auto& qe : EntityQueues_.take (e.Additional_ ["AccountID"].toString ()))
			history.append (qe.Additional_ ["History"].toList ());

		qDebug () << history.size ();

		for (const auto& lineVar : history)
		{
			const auto& histMap = lineVar.toMap ();

			const auto& origId = histMap ["EntryID"].toString ();
			QString entryId;
			if (entryIDcache.contains (origId))
				entryId = entryIDcache [origId];
			else
			{
				const auto& realId = isi->GetEntryID (origId, acc->GetQObject ());
				entryIDcache [origId] = realId;
				entryId = realId;
			}

			auto visibleName = histMap ["VisibleName"].toString ();
			if (visibleName.isEmpty ())
				visibleName = origId;

			const auto& accId = acc->GetAccountID ();

			HistoryItem item
			{
				histMap ["DateTime"].toDateTime (),
				GetDirection (histMap ["Direction"].toByteArray ()),
				histMap ["Body"].toString (),
				histMap ["OtherVariant"].toString (),
				GetMessageType (histMap ["Type"].toByteArray ()),
				histMap ["RichBody"].toString (),
				GetEscapePolicy (histMap ["EscapePolicy"].toByteArray ())
			};

			for (const auto plugin : histories)
				plugin->AddRawMessages (accId, entryId, visibleName, { item });
		}
	}

	IAccount* ImportManager::GetAccountID (Entity e)
	{
		const auto& accName = e.Additional_ ["AccountName"].toString ();

		const auto& accs = Core::Instance ().GetAccounts ([] (IProtocol *proto)
				{ return qobject_cast<ISupportImport*> (proto->GetQObject ()); });
		const auto pos = std::find_if (accs.begin (), accs.end (),
				[&accName] (IAccount *acc) { return acc->GetAccountName () == accName; });
		if (pos != accs.end ())
			return *pos;

		const auto& impId = e.Additional_ ["AccountID"].toString ();

		EntityQueues_ [impId] << e;
		if (EntityQueues_ [impId].size () > 1)
			return nullptr;

		if (AccID2OurID_.contains (impId))
			return AccID2OurID_ [impId];

		AccountHandlerChooserDialog dia (accs,
				tr ("Select account to import history from %1 into:").arg (accName));
		if (dia.exec () != QDialog::Accepted)
			return 0;

		const auto acc = dia.GetSelectedAccount ();
		AccID2OurID_ [impId] = acc;
		return acc;
	}
}
}
