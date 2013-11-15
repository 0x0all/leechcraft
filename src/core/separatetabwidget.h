/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef SEPARATETABWIDGET_H
#define SEPARATETABWIDGET_H
#include <memory>
#include <QWidget>
#include <QIcon>
#include <QTabBar>
#include <QPointer>
#include <QAction>
#include <QMap>
#include <interfaces/iinfo.h>
#include <interfaces/core/icoretabwidget.h>
#include "interfaces/core/ihookproxy.h"

class QMenu;
class QStackedWidget;
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QToolBar;

namespace LeechCraft
{
	class MainWindow;
	class SeparateTabBar;

	class SeparateTabWidget : public QWidget
							, public ICoreTabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ICoreTabWidget)

		MainWindow *Window_;

		int LastContextMenuTab_;
		QMenu *DefaultContextMenu_;
		QMenu *AddTabButtonContextMenu_;
		QPointer<QStackedWidget> MainStackedWidget_;
		QPointer<SeparateTabBar> MainTabBar_;
		QPointer<QToolButton> AddTabButton_;
		QPointer<QToolBar> LeftToolBar_;
		QPointer<QToolBar> RightToolBar_;
		QPointer<QHBoxLayout> MainTabBarLayout_;
		QPointer<QHBoxLayout> MainToolBarLayout_;
		QVBoxLayout *MainLayout_;
		QWidget *DefaultWidget_;
		QAction *AddTabButtonAction_;
		QAction *DefaultTabAction_;
		QList<QPointer<QAction>> TabBarActions_;

		QWidget *CurrentWidget_;
		int CurrentIndex_;
		QWidget *PreviousWidget_;
		QToolBar *CurrentToolBar_;

		QStringList TabNames_;

		QHash<QWidget*, QPointer<QWidget>> SavedWidgetParents_;
	public:
		explicit SeparateTabWidget (QWidget *parent = 0);
		void SetWindow (MainWindow*);

		QObject* GetQObject ();

		int WidgetCount () const;
		QWidget* Widget (int index) const;

		QList<QAction*> GetPermanentActions () const;

		QVariant TabData (int index) const;
		void SetTabData (int index, QVariant data);

		QString TabText (int index) const;
		void SetTabText (int index, const QString& text);

		QIcon TabIcon (int index) const;
		void SetTabIcon (int index, const QIcon& icon);

		QString TabToolTip (int index) const;
		void SetTabToolTip (int index, const QString& toolTip);

		QWidget* TabButton (int index, QTabBar::ButtonPosition positioin) const;
		QTabBar::ButtonPosition GetCloseButtonPosition () const;
		void SetTabClosable (int index, bool closable, QWidget *closeButton = 0);

		void SetTabsClosable (bool closable);

		void AddAction2TabBar (QAction *action);
		void InsertAction2TabBar (int position, QAction *action);
		void InsertAction2TabBar (QAction *before, QAction *action);

		void AddWidget2TabBarLayout (QTabBar::ButtonPosition pos, QWidget *action);
		void AddAction2TabBarLayout (QTabBar::ButtonPosition pos, QAction *action);
		void InsertAction2TabBarLayout (QTabBar::ButtonPosition pos, QAction *action, int index);
		void RemoveActionFromTabBarLayout (QTabBar::ButtonPosition pos, QAction *action);

		int CurrentIndex () const;
		QWidget* CurrentWidget () const;

		QMenu* GetTabMenu (int);

		int IndexOf (QWidget *page) const;

		int GetLastContextMenuTab () const;

		void SetAddTabButtonContextMenu (QMenu *menu);

		SeparateTabBar* TabBar () const;

		int AddTab (QWidget *page, const QString& text);
		int AddTab (QWidget *page, const QIcon& icon, const QString& text);
		int InsertTab (int index, QWidget *page, const QString& text);
		int InsertTab (int index, QWidget *page,
				const QIcon& icon, const QString& text);
		void RemoveTab (int index);

		void AddWidget2SeparateTabWidget (QWidget *widget);
		void RemoveWidgetFromSeparateTabWidget (QWidget *widget);

		int TabAt (const QPoint& point);

		void MoveTab (int from, int to);

		QWidget* GetPreviousWidget () const;
	protected:
		void mousePressEvent (QMouseEvent *event);
	private:
		void Init ();
		void AddTabButtonInit ();
	public slots:
		void setCurrentIndex (int index);
		void setCurrentTab (int tabIndex);
		void setCurrentWidget (QWidget *widget);
		void handleNewTabShortcutActivated ();
		void setPreviousTab ();
	private slots:
		void handleTabBarPosition ();
		void handleSelectionBehavior ();
		void handleAddDefaultTab ();
		void handleTabMoved (int from, int to);
		void handleContextMenuRequested (const QPoint& point);
		void handleActionDestroyed ();
		void releaseMouseAfterMove (int index);
	signals:
		void tabInserted (int index);
		void tabWasRemoved (int index);
		void tabCloseRequested (int index);
		void newTabMenuRequested ();
		void currentChanged (int index);
		void tabWasMoved (int from, int to);

		// Hook
		void hookTabContextMenuFill (LeechCraft::IHookProxy_ptr proxy,
				QMenu *menu, int index, int windowsId);
		void hookTabFinishedMoving (LeechCraft::IHookProxy_ptr proxy,
				int index,
				int windowId);
		void hookTabSetText (LeechCraft::IHookProxy_ptr proxy,
				int index,
				int windowId);
		void hookTabIsRemoving (LeechCraft::IHookProxy_ptr proxy,
				int index,
				int windowId);
	};
}
#endif // SEPARATETABWIDGET_H
