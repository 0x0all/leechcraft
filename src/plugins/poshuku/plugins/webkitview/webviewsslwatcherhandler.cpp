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

#include "webviewsslwatcherhandler.h"
#include <QAction>
#include <interfaces/core/iiconthememanager.h>
#include "webpagesslwatcher.h"
#include "customwebview.h"
#include "sslstatedialog.h"

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	WebViewSslWatcherHandler::WebViewSslWatcherHandler (CustomWebView *view, IIconThemeManager *itm)
	: QObject { view }
	, View_ { view }
	, SslWatcher_ { new WebPageSslWatcher { view } }
	, SslStateAction_ { new QAction { this } }
	, ITM_ { itm }
	{
		connect (SslWatcher_,
				SIGNAL (sslStateChanged (WebPageSslWatcher*)),
				this,
				SLOT (handleSslState ()));

		connect (SslStateAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (showSslDialog ()));
	}

	QAction* WebViewSslWatcherHandler::GetStateAction () const
	{
		return SslStateAction_;
	}

	void WebViewSslWatcherHandler::handleSslState ()
	{
		QString iconName;
		QString title;
		switch (SslWatcher_->GetPageState ())
		{
		case WebPageSslWatcher::State::NoSsl:
			SslStateAction_->setEnabled (false);
			return;
		case WebPageSslWatcher::State::SslErrors:
			iconName = "security-low";
			title = tr ("Some SSL errors where encountered.");
			break;
		case WebPageSslWatcher::State::UnencryptedElems:
			iconName = "security-medium";
			title = tr ("Some elements were loaded via unencrypted connection.");
			break;
		case WebPageSslWatcher::State::FullSsl:
			iconName = "security-high";
			title = tr ("Everything is secure!");
			break;
		}

		SslStateAction_->setIcon (ITM_->GetIcon (iconName));
		SslStateAction_->setText (title);
		SslStateAction_->setEnabled (true);
	}

	void WebViewSslWatcherHandler::showSslDialog ()
	{
		const auto dia = new SslStateDialog { SslWatcher_, ITM_ };
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}
}
}
}
