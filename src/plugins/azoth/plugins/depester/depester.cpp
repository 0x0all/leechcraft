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

#include "depester.h"
#include <QIcon>
#include <QAction>
#include <QTranslator>
#include <QSettings>
#include <QCoreApplication>
#include <util/util.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/icoreproxy.h>

Q_DECLARE_METATYPE (QSet<QString>);

namespace LC
{
namespace Azoth
{
namespace Depester
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("azoth_depester");
		qRegisterMetaType<QSet<QString>> ("QSet<QString>");
		qRegisterMetaTypeStreamOperators<QSet<QString>> ("QSet<QString>");

		LoadIgnores ();

		const auto iconMgr = proxy->GetIconThemeManager ();

		auto changeHandler = [this, iconMgr] { IgnoredIcon_ = iconMgr->GetIcon ("irc-unvoice"); };
		changeHandler ();

		iconMgr->RegisterChangeHandler (changeHandler);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Depester";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Depester";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows one to block messages from unwanted contacts in MUCs.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/azoth/depester/resources/images/depester.svg");
		return icon;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	bool Plugin::IsEntryIgnored (QObject *entryObj)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry ||
				entry->GetEntryType () != ICLEntry::EntryType::PrivateChat)
			return false;

		return IgnoredNicks_.contains (entry->GetEntryName ());
	}

	void Plugin::HandleMsgOccurence (IHookProxy_ptr proxy, QObject *message)
	{
		IMessage *msg = qobject_cast<IMessage*> (message);
		if (IsEntryIgnored (msg->OtherPart ()))
			proxy->CancelDefault ();
	}

	void Plugin::SaveIgnores () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Depester");
		settings.setValue ("IgnoredNicks", QVariant::fromValue (IgnoredNicks_));
	}

	void Plugin::LoadIgnores ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Depester");
		IgnoredNicks_ = settings.value ("IgnoredNicks").value<QSet<QString>> ();
	}

	void Plugin::hookEntryActionAreasRequested (IHookProxy_ptr proxy,
			QObject *action, QObject*)
	{
		if (!action->property ("Azoth/Depester/IsGood").toBool ())
			return;

		QStringList ours;
		ours << "contactListContextMenu";
		proxy->SetReturnValue (proxy->GetReturnValue ().toStringList () + ours);
	}

	void Plugin::hookEntryActionsRemoved (IHookProxy_ptr, QObject *entry)
	{
		delete Entry2ActionIgnore_.take (entry);
		Entry2Nick_.remove (entry);
	}

	void Plugin::hookEntryActionsRequested (IHookProxy_ptr proxy, QObject *entryObj)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (entry->GetEntryType () != ICLEntry::EntryType::PrivateChat)
			return;

		if (!Entry2ActionIgnore_.contains (entryObj))
		{
			QAction *action = new QAction (tr ("Ignore"), entryObj);
			action->setProperty ("Azoth/Depester/IsGood", true);
			action->setProperty ("Azoth/Depester/Entry",
					QVariant::fromValue<QObject*> (entryObj));
			action->setCheckable (true);
			action->setChecked (IsEntryIgnored (entryObj));
			connect (action,
					SIGNAL (toggled (bool)),
					this,
					SLOT (handleIgnoreEntry (bool)));
			Entry2ActionIgnore_ [entryObj] = action;
		}
		QList<QVariant> list = proxy->GetReturnValue ().toList ();
		list << QVariant::fromValue<QObject*> (Entry2ActionIgnore_ [entryObj]);
		proxy->SetReturnValue (list);
	}

	void Plugin::hookGonnaAppendMsg (LC::IHookProxy_ptr proxy,
				QObject *message)
	{
		HandleMsgOccurence (proxy, message);
	}

	void Plugin::hookGotMessage (LC::IHookProxy_ptr proxy,
				QObject *message)
	{
		HandleMsgOccurence (proxy, message);
	}

	void Plugin::hookShouldCountUnread (IHookProxy_ptr proxy,
				QObject *message)
	{
		IMessage *msg = qobject_cast<IMessage*> (message);
		if (IsEntryIgnored (msg->OtherPart ()))
		{
			proxy->CancelDefault ();
			proxy->SetReturnValue (false);
		}
	}

	void Plugin::handleIgnoreEntry (bool ignore)
	{
		QObject *entryObj = sender ()->property ("Azoth/Depester/Entry").value<QObject*> ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
			return;

		if (ignore)
		{
			const QString& nick = entry->GetEntryName ();
			IgnoredNicks_ << nick;
			Entry2Nick_ [entryObj] = nick;
			connect (entryObj,
					SIGNAL (nameChanged (const QString&)),
					this,
					SLOT (handleNameChanged (const QString&)));
		}
		else
		{
			IgnoredNicks_.remove (entry->GetEntryName ());
			Entry2Nick_.remove (entryObj);
			disconnect (entryObj,
					SIGNAL (nameChanged (const QString&)),
					this,
					SLOT (handleNameChanged (const QString&)));
		}

		SaveIgnores ();
	}

	void Plugin::hookCollectContactIcons (IHookProxy_ptr, QObject *entryObj, QList<QIcon>& icons)
	{
		const auto entry = qobject_cast<ICLEntry*> (entryObj);

		if (entry->GetEntryType () != ICLEntry::EntryType::PrivateChat)
			return;

		if (!IgnoredNicks_.contains (entry->GetEntryName ()))
			return;

		icons.prepend (IgnoredIcon_);
	}

	void Plugin::handleNameChanged (const QString& name)
	{
		QObject *entryObj = sender ();
		if (!entryObj)
			return;

		IgnoredNicks_.remove (Entry2Nick_ [entryObj]);
		IgnoredNicks_ << name;
		Entry2Nick_ [entryObj] = name;
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_depester, LC::Azoth::Depester::Plugin);
