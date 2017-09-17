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

#include <QWidget>
#include "interfaces/poshuku/iwebview.h"
#include "ui_dummywebview.h"

namespace LeechCraft
{
namespace Poshuku
{
	class DummyWebView : public QWidget
					   , public IWebView
	{
		Q_OBJECT

		Ui::DummyWebView Ui_;

		QAction * const DummyAction_;
	public:
		DummyWebView ();

		void SurroundingsInitialized () override;
		QWidget* GetQWidget () override;
		QList<QAction*> GetActions (ActionArea) const override;
		QAction* GetPageAction (PageAction) const override;
		QString GetTitle () const override;
		QUrl GetUrl () const override;
		QString GetHumanReadableUrl () const override;
		QIcon GetIcon () const override;
		void Load (const QUrl&, const QString&) override;
		void SetContent (const QByteArray&, const QByteArray&, const QUrl&) override;
		void ToHtml (const std::function<void (QString)>&) const override;
		void EvaluateJS (const QString&, const std::function<void (QVariant)>&, Util::BitFlags<EvaluateJSFlag>) override;
		void AddJavaScriptObject (const QString&, QObject*) override;
		void Print (bool) override;
		QPixmap MakeFullPageSnapshot () override;
		QPoint GetScrollPosition () const override;
		void SetScrollPosition (const QPoint&) override;
		double GetZoomFactor () const override;
		void SetZoomFactor (double) override;
		double GetTextSizeMultiplier () const override;
		void SetTextSizeMultiplier (double) override;
		QString GetDefaultTextEncoding () const override;
		void SetDefaultTextEncoding (const QString&) override;
		void InitiateFind (const QString&) override;
		QMenu* CreateStandardContextMenu () override;
		IWebViewHistory_ptr GetHistory () override;
		void SetAttribute (Attribute, bool) override;
	signals:
		void earliestViewLayout () override;
		void linkHovered (const QString&, const QString&, const QString&) override;
		void storeFormData (const PageFormsData_t&) override;
		void featurePermissionRequested (const IWebView::IFeatureSecurityOrigin_ptr&, IWebView::Feature) override;
		void zoomChanged () override;
		void closeRequested () override;
		void contextMenuRequested (const QPoint&, const ContextMenuInfo&) override;

		void loadStarted ();
		void loadProgress (int);
		void loadFinished (bool);
		void iconChanged ();
		void titleChanged (const QString&);
		void urlChanged (const QUrl&);
		void urlChanged (const QString&);
		void statusBarMessage (const QString&);
	};
}
}
