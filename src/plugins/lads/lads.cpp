/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Azer Abdullaev
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

#include "lads.h"
#include <QIcon>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QAction>
#include <QTimer>
#include <QMenuBar>
#include <QMainWindow>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/imwproxy.h>

#warning "Don't forget to add support for multiple windows here."

namespace LC
{
namespace Lads
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		UnityDetected_ = false;
		auto sb = QDBusConnection::sessionBus ();
		Action_ = 0;
		Proxy_ = proxy;
		MenuBar_ = new QMenuBar (0);
		MW_ = Proxy_->GetRootWindowsManager ()->GetMainWindow (0);
		const auto& services = sb.interface ()->registeredServiceNames ().value ();
		if (services.contains ("com.canonical.Unity"))
		{
			Action_ = new QAction (tr ("Show/hide LeechCraft window"), this);
			connect (Action_,
				SIGNAL (triggered ()),
				this,
				SLOT (showHideMain ()));
			UnityDetected_ = true;
			MW_->setMenuBar (MenuBar_);
		}
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Lads";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Lads";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Ubuntu Unity integration layer.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/resources/images/lads.svg");
		return icon;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void Plugin::hookGonnaFillMenu (IHookProxy_ptr)
	{
		if (!UnityDetected_)
			return;

		QTimer::singleShot (0,
				this,
				SLOT (fillMenu ()));
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		if (aep == ActionsEmbedPlace::TrayMenu && Action_)
			result << Action_;
		return result;
	}

	void Plugin::showHideMain () const
	{
		Proxy_->GetRootWindowsManager ()->GetMWProxy (0)->ToggleVisibility ();
	}

	void Plugin::handleGotActions (const QList<QAction*>&, ActionsEmbedPlace aep)
	{
		if (!UnityDetected_)
			return;

		if (aep != ActionsEmbedPlace::ToolsMenu)
			return;

		MenuBar_->clear ();
		QTimer::singleShot (0,
				this,
				SLOT (fillMenu ()));
	}

	void Plugin::fillMenu ()
	{
		if (!UnityDetected_)
			return;

		auto menu = Proxy_->GetRootWindowsManager ()->GetMWProxy (0)->GetMainMenu ();

		QMenu *lcMenu = 0;
		QList<QAction*> firstLevelActions;
		for (auto action : menu->actions ())
			if (action->menu ())
			{
				MenuBar_->addAction (action);
				if (!lcMenu)
					lcMenu = action->menu ();
			}
			else
			{
				if (action->menuRole () == QAction::TextHeuristicRole)
					action->setMenuRole (QAction::ApplicationSpecificRole);
				firstLevelActions << action;
			}

		if (!lcMenu)
		{
			qWarning () << Q_FUNC_INFO
					<< "LeechCraft menu not found";
			return;
		}

		for (auto act : irstLevelActions)
			lcMenu->addAction (act);

		if (!lcMenu->actions ().isEmpty ())
			MenuBar_->addMenu (lcMenu);

		for (auto actor : Proxy_->GetPluginsManager ()->GetAllCastableRoots<IActionsExporter*> ())
			connect (actor,
					SIGNAL (gotActions (QList<QAction*>, LC::ActionsEmbedPlace)),
					this,
					SLOT (handleGotActions (QList<QAction*>, LC::ActionsEmbedPlace)));
	}

}
}

LC_EXPORT_PLUGIN (leechcraft_lads, LC::Lads::Plugin);

