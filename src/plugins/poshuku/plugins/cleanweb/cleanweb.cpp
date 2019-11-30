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

#include "cleanweb.h"
#include <QIcon>
#include <QTextCodec>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/poshuku/ibrowserwidget.h>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "subscriptionsmanagerwidget.h"
#include "userfilters.h"
#include "wizardgenerator.h"
#include "subscriptionsmodel.h"
#include "userfiltersmodel.h"

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	void CleanWeb::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("poshuku_cleanweb");

		SettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"poshukucleanwebsettings.xml");

		const auto model = new SubscriptionsModel { this };
		const auto ufm = new UserFiltersModel { proxy, this };
		Core_ = std::make_shared<Core> (model, ufm, proxy);

		SettingsDialog_->SetCustomWidget ("SubscriptionsManager",
				new SubscriptionsManagerWidget (Core_.get (), model));
		SettingsDialog_->SetCustomWidget ("UserFilters", new UserFilters (ufm));
	}

	void CleanWeb::SecondInit ()
	{
		Core_->InstallInterceptor ();
	}

	void CleanWeb::Release ()
	{
	}

	QByteArray CleanWeb::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.CLeanWeb";
	}

	QString CleanWeb::GetName () const
	{
		return "Poshuku CleanWeb";
	}

	QString CleanWeb::GetInfo () const
	{
		return tr ("Blocks unwanted ads.");
	}

	QIcon CleanWeb::GetIcon () const
	{
		static QIcon icon ("lcicons:/plugins/poshuku/plugins/cleanweb/resources/images/poshuku_cleanweb.svg");
		return icon;
	}

	QStringList CleanWeb::Needs () const
	{
		return { "http" };
	}

	Util::XmlSettingsDialog_ptr CleanWeb::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	EntityTestHandleResult CleanWeb::CouldHandle (const Entity& e) const
	{
		return Core_->CouldHandle (e) ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void CleanWeb::Handle (Entity e)
	{
		Core_->Handle (e);
	}

	QList<QWizardPage*> CleanWeb::GetWizardPages () const
	{
		return WizardGenerator::GetPages (Core_.get ());
	}

	QSet<QByteArray> CleanWeb::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void CleanWeb::hookBrowserWidgetInitialized (IHookProxy_ptr, QObject *browserWidget)
	{
		Core_->HandleBrowserWidget (qobject_cast<IBrowserWidget*> (browserWidget));
	}

	void CleanWeb::hookWebViewContextMenu (IHookProxy_ptr,
			IWebView *view,
			const ContextMenuInfo& r,
			QMenu *menu,
			WebViewCtxMenuStage stage)
	{
		Core_->HandleContextMenu (r, view, menu, stage);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_cleanweb, LC::Poshuku::CleanWeb::CleanWeb);
