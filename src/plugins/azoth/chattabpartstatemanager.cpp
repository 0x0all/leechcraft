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

#include "chattabpartstatemanager.h"
#include <QTimer>
#include "interfaces/azoth/iclentry.h"
#include "chattab.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	ChatTabPartStateManager::ChatTabPartStateManager (ChatTab *tab)
	: QObject { tab }
	, Tab_ { tab }
	, TypeTimer_ { new QTimer { this } }
	{
		auto setState = [this] (ChatPartState st)
		{
			return [this, st] { SetChatPartState (st); };
		};

		connect (tab,
				&ChatTab::entryLostCurrent,
				this,
				setState (CPSInactive));
		connect (tab,
				&ChatTab::hookMessageSendRequested,
				this,
				setState (CPSActive));

		connect (tab,
				&ChatTab::composingTextChanged,
				this,
				&ChatTabPartStateManager::HandleComposingText);

		TypeTimer_->setInterval (2000);
		connect (TypeTimer_,
				&QTimer::timeout,
				this,
				setState (CPSPaused));
		connect (tab,
				&ChatTab::entryLostCurrent,
				TypeTimer_,
				&QTimer::stop);

		connect (tab,
				&ChatTab::currentVariantChanged,
				this,
				&ChatTabPartStateManager::HandleVariantChanged);
	}

	ChatTabPartStateManager::~ChatTabPartStateManager ()
	{
		SetChatPartState (CPSGone);
	}

	void ChatTabPartStateManager::SetChatPartState (ChatPartState state)
	{
		if (state == PreviousState_)
			return;

		if (!XmlSettingsManager::Instance ().property ("SendChatStates").toBool ())
			return;

		auto entry = Tab_->GetICLEntry ();
		if (!entry)
			return;

		PreviousState_ = state;

		if (state != CPSGone ||
				XmlSettingsManager::Instance ().property ("SendEndConversations").toBool ())
			entry->SetChatPartState (state, LastVariant_);
	}

	void ChatTabPartStateManager::HandleComposingText (const QString& text)
	{
		TypeTimer_->stop ();

		if (!text.isEmpty ())
		{
			SetChatPartState (CPSComposing);
			TypeTimer_->start ();
		}
	}

	void ChatTabPartStateManager::HandleVariantChanged (const QString& variant)
	{
		if (LastVariant_.isEmpty () || variant == LastVariant_)
			return;

		if (!XmlSettingsManager::Instance ().property ("SendChatStates").toBool ())
			return;

		auto entry = Tab_->GetICLEntry ();
		if (entry && entry->GetStatus (LastVariant_).State_ != SOffline)
			entry->SetChatPartState (CPSInactive, LastVariant_);

		LastVariant_ = variant;
	}
}
}
