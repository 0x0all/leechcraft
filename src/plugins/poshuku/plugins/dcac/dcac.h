/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/poshuku/poshukutypes.h>

class QContextMenuEvent;

namespace LC
{
namespace Poshuku
{
class IWebView;

namespace DCAC
{
	class ViewsManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku.DCAC")

		ViewsManager *ViewsManager_ = nullptr;
		Util::XmlSettingsDialog_ptr XSD_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	public slots:
		void hookBrowserWidgetInitialized (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookWebViewContextMenu (LC::IHookProxy_ptr proxy,
				LC::Poshuku::IWebView *view,
				const LC::Poshuku::ContextMenuInfo& info,
				QMenu *menu,
				WebViewCtxMenuStage menuBuildStage);
	};
}
}
}

