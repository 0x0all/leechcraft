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

#include "sourceviewer.h"
#include <QDesktopWidget>
#include <util/gui/findnotification.h>
#include <interfaces/core/icoreproxy.h>
#include "htmlhighlighter.h"
#include "core.h"

namespace LeechCraft
{
namespace Poshuku
{
	namespace
	{
		class SourceFinder : public Util::FindNotification
		{
			QTextEdit * const Edit_;
		public:
			SourceFinder (QTextEdit *edit, const ICoreProxy_ptr& proxy)
			: FindNotification { proxy, edit }
			, Edit_ { edit }
			{
			}
		protected:
			void handleNext (const QString& text, FindFlags flags)
			{
				QTextDocument::FindFlags tdFlags;
				if (flags & FindCaseSensitively)
					tdFlags |= QTextDocument::FindCaseSensitively;
				if (flags & FindBackwards)
					tdFlags |= QTextDocument::FindWholeWords;
				Edit_->find (text, tdFlags);
			}
		};
	}

	SourceViewer::SourceViewer (QWidget *parent)
	: QMainWindow { parent }
	{
		Ui_.setupUi (this);

		auto frect = frameGeometry ();
		frect.moveCenter (QDesktopWidget ().availableGeometry ().center ());
		move (frect.topLeft ());
		new HtmlHighlighter { Ui_.HtmlEdit_ };

		Finder_ = new SourceFinder
		{
			Ui_.HtmlEdit_,
			Core::Instance ().GetProxy ()
		};
	}

	void SourceViewer::SetHtml (const QString& html)
	{
		Ui_.HtmlEdit_->setPlainText (html);
	}
}
}
