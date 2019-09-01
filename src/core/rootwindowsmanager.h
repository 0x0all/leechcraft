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

#include <functional>
#include <QObject>
#include <interfaces/core/irootwindowsmanager.h>
#include "mainwindow.h"

class QIcon;
class QScreen;

namespace LeechCraft
{
	class TabManager;
	class MWProxy;

	class RootWindowsManager : public QObject
							 , public IRootWindowsManager
	{
		Q_OBJECT
		Q_INTERFACES (IRootWindowsManager)

		struct WinData
		{
			MainWindow *Window_;
			MWProxy *Proxy_;
			TabManager *TM_;
		};
		QList<WinData> Windows_;

		bool IsShuttingDown_ = false;
	public:
		RootWindowsManager (QObject* = nullptr);

		void Initialize ();
		void Release ();

		MainWindow* MakeMainWindow ();
		TabManager* GetTabManager (MainWindow*) const;
		TabManager* GetTabManager (int) const;

		bool WindowCloseRequested (MainWindow*);

		QObject* GetQObject () override;

		int GetWindowsCount () const override;
		int GetPreferredWindowIndex () const override;
		int GetPreferredWindowIndex (const ITabWidget*) const;
		int GetPreferredWindowIndex (const QByteArray&) const override;
		int GetWindowForTab (ITabWidget*) const override;
		int GetWindowIndex (QMainWindow*) const override;

		IMWProxy* GetMWProxy (int) const override;
		MainWindow* GetMainWindow (int) const override;
		ICoreTabWidget* GetTabWidget (int) const override;
	private:
		MainWindow* CreateWindow (QScreen *screen, bool primary);
		void PerformWithTab (const std::function<void (TabManager*, int)>&, QWidget*);
		void MoveTab (int tab, int fromWin, int toWin);
		void CloseWindowTabs (int index);
	public slots:
		void moveTabToNewWindow ();
		void moveTabToExistingWindow ();

		void add (const QString&, QWidget*);
		void remove (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void bringToFront (QWidget*);
	signals:
		void windowAdded (int) override;
		void windowRemoved (int) override;
		void currentWindowChanged (int, int) override;
		void tabAdded (int, int) override;
		void tabIsRemoving (int, int) override;
		void tabIsMoving (int, int, int) override;
		void tabMoved (int, int, int) override;

		void hookTabAdding (LeechCraft::IHookProxy_ptr, QWidget*);
		void hookGetPreferredWindowIndex (LeechCraft::IHookProxy_ptr, const QWidget*) const;
		void hookGetPreferredWindowIndex (LeechCraft::IHookProxy_ptr, const QByteArray&) const;
		void hookGetPreferredWindowIndex (LeechCraft::IHookProxy_ptr) const;
	};
}
