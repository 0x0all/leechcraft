/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chattabwebview.h"
#include <QContextMenuEvent>
#include <QWebHitTestResult>
#include <QPointer>
#include <QMenu>
#include <QDesktopServices>
#include <QUrlQuery>
#include <util/xpc/util.h>
#include <util/xpc/stddatafiltermenucreator.h>
#include <util/sll/util.h>
#include <interfaces/idatafilter.h>
#include <interfaces/core/icoreproxy.h>
#include "interfaces/azoth/iclentry.h"
#include "core.h"
#include "actionsmanager.h"

namespace LC
{
namespace Azoth
{
	ChatTabWebView::ChatTabWebView (QWidget *parent)
	: QWebView (parent)
	, QuoteAct_ (0)
	{
		connect (page (),
				SIGNAL (linkClicked (QUrl)),
				this,
				SLOT (handlePageLinkClicked (QUrl)));
	}

	void ChatTabWebView::SetQuoteAction (QAction *act)
	{
		QuoteAct_ = act;
	}

	void ChatTabWebView::mouseReleaseEvent (QMouseEvent *e)
	{
		if (e->button () != Qt::MiddleButton)
			return QWebView::mouseReleaseEvent (e);

		const auto r = page ()->mainFrame ()->hitTestContent (e->pos ());
		if (r.linkUrl ().isEmpty ())
			return QWebView::mouseReleaseEvent (e);

		emit linkClicked (r.linkUrl (), false);
	}

	void ChatTabWebView::contextMenuEvent (QContextMenuEvent *e)
	{
		QPointer<QMenu> menu (new QMenu (this));
		const auto menuGuard = Util::MakeScopeGuard ([&menu] { delete menu; });

		const auto r = page ()->mainFrame ()->hitTestContent (e->pos ());

		if (!r.linkUrl ().isEmpty ())
		{
			if (r.linkUrl ().scheme () == "azoth")
				HandleNick (menu, r.linkUrl ());
			else
				HandleURL (menu, r.linkUrl ());
		}

		const auto& text = page ()->selectedText ();
		if (!text.isEmpty ())
		{
			menu->addAction (pageAction (QWebPage::Copy));
			menu->addAction (QuoteAct_);

			if (!text.contains (' ') && text.contains ('.'))
				menu->addAction (tr ("Open as URL"),
						this,
						SLOT (handleOpenAsURL ()))->setData (text);

			menu->addSeparator ();

			menu->addAction (tr ("Highlight all occurrences"),
					this,
					SLOT (handleHighlightOccurences ()))->setData (text);

			menu->addSeparator ();

			HandleDataFilters (menu, text);
		}

		if (!r.imageUrl ().isEmpty ())
			menu->addAction (pageAction (QWebPage::CopyImageToClipboard));

		if (settings ()->testAttribute (QWebSettings::DeveloperExtrasEnabled))
			menu->addAction (pageAction (QWebPage::InspectElement));

		if (menu->isEmpty ())
			return;

		menu->exec (mapToGlobal (e->pos ()));
	}

	void ChatTabWebView::HandleNick (QMenu *menu, const QUrl& nickUrl)
	{
		const auto& entryIdValue = QUrlQuery { nickUrl }.queryItemValue ("entryId");
		const auto& entryId = QUrl::fromPercentEncoding (entryIdValue.toUtf8 ());
		if (entryId.isEmpty ())
			return;

		const auto entry = qobject_cast<ICLEntry*> (Core::Instance ().GetEntry (entryId));
		if (!entry)
			return;

		QList<QAction*> actions;

		const auto manager = Core::Instance ().GetActionsManager ();
		for (const auto act : manager->GetEntryActions (entry))
			if (manager->GetAreasForAction (act).contains (ActionsManager::CLEAAChatCtxtMenu))
				actions << act;

		menu->addActions (actions);

		menu->addAction (tr ("Highlight the nickname"),
				this,
				SLOT (handleHighlightOccurences ()))->setData (entry->GetEntryName ());
	}

	void ChatTabWebView::HandleURL (QMenu *menu, const QUrl& url)
	{
		menu->addAction (tr ("Open"),
				this,
				SLOT (handleOpenLink ()))->setData (url);
		menu->addAction (tr ("Save..."),
				this,
				SLOT (handleSaveLink ()))->setData (url);
		menu->addAction (tr ("Open externally"),
				this,
				SLOT (handleOpenExternally ()))->setData (url);
		menu->addAction (pageAction (QWebPage::CopyLinkToClipboard));
		menu->addSeparator ();
	}

	void ChatTabWebView::HandleDataFilters (QMenu *menu, const QString& text)
	{
		new Util::StdDataFilterMenuCreator (text,
				Core::Instance ().GetProxy ()->GetEntityManager (), menu);
	}

	void ChatTabWebView::handleOpenLink ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		const Entity& e = Util::MakeEntity (action->data (),
				{},
				OnlyHandle | FromUserInitiated);
		Core::Instance ().SendEntity (e);
	}

	void ChatTabWebView::handleOpenExternally ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		const QUrl& url = action->data ().toUrl ();
		if (url.isEmpty ())
			return;

		QDesktopServices::openUrl (url);
	}

	void ChatTabWebView::handleOpenAsURL()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		const auto& str = action->data ().toString ().trimmed ();

		QUrl url (str);
		if (url.scheme ().isEmpty () &&
					url.host ().isEmpty ())
			url = "http://" + url.toString ();

		const Entity& e = Util::MakeEntity (url,
				{},
				OnlyHandle | FromUserInitiated);
		Core::Instance ().SendEntity (e);
	}

	void ChatTabWebView::handleHighlightOccurences ()
	{
		const auto action = qobject_cast<QAction*> (sender ());
		emit chatWindowSearchRequested (action->data ().toString ().trimmed ());
	}

	void ChatTabWebView::handleSaveLink ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		Entity e = Util::MakeEntity (action->data (),
				QString (),
				FromUserInitiated);
		e.Additional_ ["AllowedSemantics"] = QStringList ("fetch") << "save";
		Core::Instance ().SendEntity (e);
	}

	void ChatTabWebView::handlePageLinkClicked (const QUrl& url)
	{
		emit linkClicked (url, true);
	}
}
}
