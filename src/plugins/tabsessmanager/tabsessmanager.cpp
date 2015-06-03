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

#include "tabsessmanager.h"
#include <algorithm>
#include <QIcon>
#include <QTimer>
#include <QSettings>
#include <QApplication>
#include <QtDebug>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/ihavetabs.h>
#include "sessionmenumanager.h"
#include "sessionsmanager.h"
#include "unclosemanager.h"

namespace LeechCraft
{
namespace TabSessManager
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("tabsessmanager");

		UncloseMgr_ = new UncloseManager { proxy };

		SessionsMgr_ = new SessionsManager { proxy };

		SessionMenuMgr_ = new SessionMenuManager;
		connect (SessionMenuMgr_,
				SIGNAL (loadRequested (QString)),
				SessionsMgr_,
				SLOT (loadCustomSession (QString)));
		connect (SessionMenuMgr_,
				SIGNAL (addRequested (QString)),
				SessionsMgr_,
				SLOT (addCustomSession (QString)));
		connect (SessionMenuMgr_,
				SIGNAL (deleteRequested (QString)),
				SessionsMgr_,
				SLOT (deleteCustomSession (QString)));
		connect (SessionMenuMgr_,
				SIGNAL (saveCustomSessionRequested ()),
				SessionsMgr_,
				SLOT (saveCustomSession ()));

		connect (SessionsMgr_,
				SIGNAL (gotCustomSession (QString)),
				SessionMenuMgr_,
				SLOT (addCustomSession (QString)));

		Proxy_ = proxy;

		for (const auto& name : SessionsMgr_->GetCustomSessions ())
			SessionMenuMgr_->addCustomSession (name);
	}

	void Plugin::SecondInit ()
	{
		QTimer::singleShot (5000,
				SessionsMgr_,
				SLOT (recover ()));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.TabSessManager";
	}

	void Plugin::Release ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager");
		settings.setValue ("CleanShutdown", true);
	}

	QString Plugin::GetName () const
	{
		return "TabSessManager";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Manages sessions of tabs in LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		switch (place)
		{
		case ActionsEmbedPlace::ToolsMenu:
			return
			{
				SessionMenuMgr_->GetSessionsAction (),
				UncloseMgr_->GetMenuAction ()
			};
		case ActionsEmbedPlace::CommonContextMenu:
			return { UncloseMgr_->GetMenuAction () };
		default:
			return {};
		}
	}

	void Plugin::hookTabIsRemoving (IHookProxy_ptr, int index, int windowId)
	{
		const auto rootWM = Proxy_->GetRootWindowsManager ();
		const auto tabWidget = rootWM->GetTabWidget (windowId);
		const auto widget = tabWidget->Widget (index);

		UncloseMgr_->HandleRemoveTab (widget);
		SessionsMgr_->handleRemoveTab (widget);
	}

	void Plugin::hookTabAdding (IHookProxy_ptr, QWidget *widget)
	{
		SessionsMgr_->HandleTabAdding (widget);
	}

	void Plugin::hookGetPreferredWindowIndex (IHookProxy_ptr proxy, const QWidget *widget) const
	{
		SessionsMgr_->HandlePreferredWindowIndex (proxy, widget);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_tabsessmanager, LeechCraft::TabSessManager::Plugin);
