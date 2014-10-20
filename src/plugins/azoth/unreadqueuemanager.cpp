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

#include "unreadqueuemanager.h"
#include <QMainWindow>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "interfaces/azoth/iclentry.h"
#include "core.h"
#include "chattabsmanager.h"

uint qHash (const QPointer<QObject>& ptr)
{
	return qHash (ptr.data ());
}

namespace LeechCraft
{
namespace Azoth
{
	UnreadQueueManager::UnreadQueueManager (QObject *parent)
	: QObject (parent)
	{
	}

	QObject* UnreadQueueManager::GetFirstUnreadMessage (QObject* entryObj) const
	{
		return Entry2FirstUnread_.value (entryObj);
	}

	void UnreadQueueManager::AddMessage (QObject *msgObj)
	{
		const auto msg = qobject_cast<IMessage*> (msgObj);
		const auto entryObj = msg->ParentCLEntry ();
		if (!Queue_.contains (entryObj))
		{
			Queue_ << entryObj;
			Entry2FirstUnread_ [entryObj] = msgObj;
		}

		UnreadMessages_ << msgObj;
	}

	bool UnreadQueueManager::IsMessageRead (QObject *msgObj) const
	{
		return !UnreadMessages_.contains (msgObj);
	}

	void UnreadQueueManager::ShowNext ()
	{
		QObject *entryObj = nullptr;
		while (!Queue_.isEmpty () && !entryObj)
			entryObj = Queue_.takeFirst ();
		if (!entryObj)
			return;

		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto chatWidget = Core::Instance ().GetChatTabsManager ()->OpenChat (entry, true);

		const auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
		const auto idx = rootWM->GetWindowForTab (qobject_cast<ITabWidget*> (chatWidget));
		if (const auto mw = rootWM->GetMainWindow (idx))
		{
			mw->show ();
			mw->raise ();
			mw->activateWindow ();
		}
	}

	void UnreadQueueManager::clearMessagesForEntry (QObject *entryObj)
	{
		Queue_.removeAll (entryObj);

		for (auto pos = UnreadMessages_.begin (); pos != UnreadMessages_.end (); )
			if (!*pos || qobject_cast<IMessage*> (*pos)->ParentCLEntry () == entryObj)
				pos = UnreadMessages_.erase (pos);
			else
				++pos;

		emit messagesCleared (entryObj);
	}
}
}
