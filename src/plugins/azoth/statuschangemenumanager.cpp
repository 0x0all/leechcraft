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

#include "statuschangemenumanager.h"
#include <QMenu>
#include "core.h"
#include "customstatusesmanager.h"
#include "resourcesmanager.h"

namespace LC
{
namespace Azoth
{
	StatusChangeMenuManager::StatusChangeMenuManager (QObject *parent)
	: QObject (parent)
	{
	}

	QMenu* StatusChangeMenuManager::CreateMenu (QObject* obj, const char* slot, QWidget *parent, bool autoupdate)
	{
		QMenu *result = new QMenu (tr ("Change status"), parent);
		result->addAction (ResourcesManager::Instance ().GetIconForState (SOnline),
				tr ("Online"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SOnline));
		result ->addAction (ResourcesManager::Instance ().GetIconForState (SChat),
				tr ("Free to chat"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SChat));
		result ->addAction (ResourcesManager::Instance ().GetIconForState (SAway),
				tr ("Away"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SAway));
		result ->addAction (ResourcesManager::Instance ().GetIconForState (SDND),
				tr ("DND"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SDND));
		result ->addAction (ResourcesManager::Instance ().GetIconForState (SXA),
				tr ("Not available"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SXA));
		result ->addAction (ResourcesManager::Instance ().GetIconForState (SOffline),
				tr ("Offline"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SOffline));

		result->addSeparator ();
		auto customAct = result->addAction (QString (), obj, slot);

		if (autoupdate)
			connect (result,
					SIGNAL (aboutToShow ()),
					this,
					SLOT (updateCustomStatuses ()));
		connect (result,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleMenuDestroyed ()));

		Infos_ [result] = { obj, slot, customAct };

		return result;
	}

	void StatusChangeMenuManager::UpdateCustomStatuses (QMenu *rootMenu)
	{
		if (!Infos_.contains (rootMenu))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown menu"
					<< rootMenu
					<< Infos_.keys ();
			return;
		}

		const auto& info = Infos_ [rootMenu];

		std::shared_ptr<QMenu> oldMenu (info.CustomAction_->menu ());

		const auto& customs = Core::Instance ().GetCustomStatusesManager ()->GetStates ();
		if (customs.isEmpty ())
		{
			info.CustomAction_->setText (tr ("Custom..."));
			info.CustomAction_->setMenu (nullptr);
			return;
		}

		auto menu = new QMenu;
		for (const auto& custom : customs)
		{
			auto action = menu->addAction (custom.Name_, info.Obj_, info.Slot_);
			action->setIcon (ResourcesManager::Instance ().GetIconForState (custom.State_));
			action->setProperty ("Azoth/TargetState", QVariant::fromValue<State> (custom.State_));
			action->setProperty ("Azoth/TargetText", custom.Text_);
		}

		menu->addSeparator ();

		menu->addAction (tr ("Other..."), info.Obj_, info.Slot_);

		info.CustomAction_->setText (tr ("Custom"));
		info.CustomAction_->setMenu (menu);
	}

	void StatusChangeMenuManager::updateCustomStatuses ()
	{
		UpdateCustomStatuses (static_cast<QMenu*> (sender ()));
	}

	void StatusChangeMenuManager::handleMenuDestroyed ()
	{
		const auto& info = Infos_.take (sender ());
		if (info.CustomAction_)
		{
			auto menu = info.CustomAction_->menu ();
			info.CustomAction_->setMenu (nullptr);
			delete menu;
		}
	}
}
}
