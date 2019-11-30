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

#include "dummywebview.h"
#include <QIcon>
#include <QAction>

namespace LC
{
namespace Poshuku
{
	DummyWebView::DummyWebView ()
	: DummyAction_ { new QAction { this } }
	{
		Ui_.setupUi (this);
	}

	void DummyWebView::SurroundingsInitialized ()
	{
	}

	QWidget* DummyWebView::GetQWidget ()
	{
		return this;
	}

	QList<QAction*> DummyWebView::GetActions (IWebView::ActionArea) const
	{
		return {};
	}

	QAction* DummyWebView::GetPageAction (IWebView::PageAction) const
	{
		return DummyAction_;
	}

	QString DummyWebView::GetTitle () const
	{
		return {};
	}

	QUrl DummyWebView::GetUrl () const
	{
		return {};
	}

	QString DummyWebView::GetHumanReadableUrl () const
	{
		return {};
	}

	QIcon DummyWebView::GetIcon () const
	{
		return {};
	}

	void DummyWebView::Load (const QUrl&, const QString&)
	{
	}

	void DummyWebView::SetContent (const QByteArray&, const QByteArray&, const QUrl&)
	{
	}

	void DummyWebView::ToHtml (const std::function<void (QString)>& function) const
	{
		function ({});
	}

	void DummyWebView::EvaluateJS (const QString&,
			const std::function<void (QVariant)>&,
			Util::BitFlags<IWebView::EvaluateJSFlag>)
	{
	}

	void DummyWebView::AddJavaScriptObject (const QString&, QObject*)
	{
	}

	void DummyWebView::Print (bool)
	{
	}

	QPixmap DummyWebView::MakeFullPageSnapshot ()
	{
		return {};
	}

	QPoint DummyWebView::GetScrollPosition () const
	{
		return {};
	}

	void DummyWebView::SetScrollPosition (const QPoint&)
	{
	}

	double DummyWebView::GetZoomFactor () const
	{
		return 1;
	}

	void DummyWebView::SetZoomFactor (double)
	{
	}

	QString DummyWebView::GetDefaultTextEncoding () const
	{
		return {};
	}

	void DummyWebView::SetDefaultTextEncoding (const QString&)
	{
	}

	void DummyWebView::InitiateFind (const QString&)
	{
	}

	QMenu* DummyWebView::CreateStandardContextMenu ()
	{
		return nullptr;
	}

	IWebViewHistory_ptr DummyWebView::GetHistory ()
	{
		return {};
	}

	void DummyWebView::SetAttribute (IWebView::Attribute, bool)
	{
	}
}
}
