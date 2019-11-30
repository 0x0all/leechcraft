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

#include "xproxy.h"
#include <QIcon>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include "proxyfactory.h"
#include "proxiesconfigwidget.h"
#include "xmlsettingsmanager.h"
#include "proxiesstorage.h"
#include "scriptsmanager.h"
#include "urllistscript.h"

namespace LC
{
namespace XProxy
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("xproxy");

		CoreProxy_ = proxy;

		qRegisterMetaTypeStreamOperators<Proxy> ("LC::XProxy::Proxy");
		qRegisterMetaTypeStreamOperators<ReqTarget> ("LC::XProxy::ReqTarget");
		qRegisterMetaTypeStreamOperators<QList<LC::XProxy::Entry_t>> ("QList<LC::XProxy::Entry_t>");
		qRegisterMetaTypeStreamOperators<QList<LC::XProxy::ScriptEntry_t>> ("QList<LC::XProxy::ScriptEntry_t>");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "xproxysettings.xml");

		const auto scriptsMgr = new ScriptsManager { proxy };
		Storage_ = new ProxiesStorage { scriptsMgr} ;

		CfgWidget_ = new ProxiesConfigWidget { Storage_, scriptsMgr };
		XSD_->SetCustomWidget ("Proxies", CfgWidget_);

		XmlSettingsManager::Instance ().RegisterObject ("EnableForNAM", this, "handleReenable");
		XmlSettingsManager::Instance ().RegisterObject ("EnableForApp", this, "handleReenable");
		handleReenable ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.XProxy";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "XProxy";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Advanced proxy servers manager for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::handleReenable ()
	{
		const bool app = XmlSettingsManager::Instance ().property ("EnableForApp").toBool ();
		QNetworkProxyFactory::setApplicationProxyFactory (app ? new ProxyFactory (Storage_) : 0);

		const bool nam = XmlSettingsManager::Instance ().property ("EnableForNAM").toBool ();
		CoreProxy_->GetNetworkAccessManager ()->setProxyFactory (nam ? new ProxyFactory (Storage_) : 0);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_xproxy, LC::XProxy::Plugin);
