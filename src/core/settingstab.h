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

#include <memory>
#include <QStack>
#include <QWidget>
#include "interfaces/ihavetabs.h"
#include "ui_settingstab.h"

class IHaveSettings;
class QLineEdit;
class QToolButton;

namespace LeechCraft
{
	class SettingsWidget;

	class SettingsTab : public QWidget
					  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::SettingsTab Ui_;
		QToolBar *Toolbar_;
		QAction *ActionBack_;
		QAction *ActionApply_;
		QAction *ActionCancel_;

		QString LastSearch_;
		QHash<QToolButton*, QObject*> Button2SettableRoot_;
		QHash<IHaveSettings*, QList<int>> Obj2SearchMatchingPages_;

		QStack<std::shared_ptr<SettingsWidget>> SettingsWidgets_;
	public:
		SettingsTab (QWidget* = 0);

		void Initialize ();

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	public slots:
		void showSettingsFor (QObject*);
	private slots:
		void addSearchBox ();
		void handleSearch (const QString&);

		void handleBackRequested ();
		void handleApply ();
		void handleCancel ();
	signals:
		void remove (QWidget*);
	};
}
