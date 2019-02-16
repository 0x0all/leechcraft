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

#include "readitlater.h"
#include <QIcon>
#include <util/util.h>
#include "readitlaterauthwidget.h"
#include "readitlaterservice.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("poshuku_onlinebookmarks_readitlater");

		ReadItLaterService_ = std::make_shared<ReadItLaterService> (proxy);
	}

	void Plugin::SecondInit ()
	{
		ReadItLaterService_->Prepare ();
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.OnlineBookmarks.ReadItLater";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku OB: Read It Later";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Sync local bookmarks with your Read It Later account.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/poshuku/onlinebookmarks:readitlater/resources/images/readitlater.svg");
		return icon;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Plugins.Poshuku.Plugins.OnlineBookmarks.IServicePlugin" };
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QObject* Plugin::GetBookmarksService () const
	{
		return ReadItLaterService_.get ();
	}

}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_onlinebookmarks_readitlater,
		LeechCraft::Poshuku::OnlineBookmarks::ReadItLater::Plugin);
