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

#pragma once

#include <QObject>
#include <QMainWindow>
#include <QToolBar>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ihaveshortcuts.h>

class QDockWidget;

namespace LC
{
namespace Util
{
	class ShortcutManager;
}

namespace SB2
{
	class ViewManager;
	class TrayComponent;
	class LauncherComponent;
	class DockActionComponent;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveShortcuts
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveShortcuts)

		LC_PLUGIN_METADATA ("org.LeechCraft.SB2")

		ICoreProxy_ptr Proxy_;

		struct WindowInfo
		{
			std::shared_ptr<ViewManager> Mgr_;
			std::shared_ptr<TrayComponent> Tray_;
			std::shared_ptr<LauncherComponent> Launcher_;
			std::shared_ptr<DockActionComponent> Dock_;
		};
		QList<WindowInfo> Managers_;

		Util::ShortcutManager *ShortcutMgr_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		QMap<QString, ActionInfo> GetActionInfo () const override;
		void SetShortcut (const QString&, const QKeySequences_t&) override;
	public slots:
		void hookGonnaShowStatusBar (LC::IHookProxy_ptr, bool);
		void hookDockWidgetActionVisToggled (LC::IHookProxy_ptr,
				QMainWindow*, QDockWidget*, bool);
		void hookAddingDockAction (LC::IHookProxy_ptr,
				QMainWindow*, QAction*, Qt::DockWidgetArea);
		void hookRemovingDockAction (LC::IHookProxy_ptr,
				QMainWindow*, QAction*, Qt::DockWidgetArea);
		void hookDockBarWillBeShown (LC::IHookProxy_ptr,
				QMainWindow*, QToolBar*, Qt::DockWidgetArea);
	private slots:
		void handleWindow (int, bool init = false);
		void handleWindowRemoved (int);
	signals:
		void pluginsAvailable ();
	};
}
}

