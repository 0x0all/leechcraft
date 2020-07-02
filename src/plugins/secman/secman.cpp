/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Vadim Misbakh-Soloviev, Georg Rudoy
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

#include "secman.h"
#include <QIcon>
#include <QAction>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "contentsdisplaydialog.h"
#include "persistentstorage.h"

namespace LC
{
namespace SecMan
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("secman");

#ifdef SECMAN_EXPOSE_CONTENTSDISPLAY
		auto displayContentsAction = new QAction (tr ("Display storages' contents"), this);
		connect (displayContentsAction,
				SIGNAL (triggered ()),
				this,
				SLOT (handleDisplayContents ()));
		MenuActions_ ["tools"] << displayContentsAction;
#endif
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.SecMan";
	}

	QString Plugin::GetName () const
	{
		return "SecMan";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Secure data storage for other LeechCraft modules.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		return Core::Instance ().GetExpectedPluginClasses ();
	}

	void Plugin::AddPlugin (QObject *plugin)
	{
		Core::Instance ().AddPlugin (plugin);
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace) const
	{
		return {};
	}

	QMap<QString, QList<QAction*>> Plugin::GetMenuActions () const
	{
		return MenuActions_;
	}

	IPersistentStorage_ptr Plugin::RequestStorage ()
	{
		return std::make_shared<PersistentStorage> ();
	}

	void Plugin::handleDisplayContents ()
	{
		auto dia = new ContentsDisplayDialog;
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_secman, LC::SecMan::Plugin);
