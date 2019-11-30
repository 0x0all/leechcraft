/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
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

#include "dropbox.h"
#include <QIcon>
#include <QSettings>
#include <QCoreApplication>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <util/sll/prelude.h>
#include "authmanager.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace NetStoreManager
{
namespace DBox
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("netstoremanager_googledrive");

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog);
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"nsmgoogledrivesettings.xml");

		Core::Instance ().SetProxy (proxy);
		AuthManager_ = new AuthManager (this);

		connect (AuthManager_,
				SIGNAL (authSuccess (QObject*)),
				this,
				SLOT (handleAuthSuccess (QObject*)));

		ReadAccounts ();
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.NetStoreManager.DBox";
	}

	QString Plugin::GetName () const
	{
		return "NetStoreManager: DBox";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for DropBox for NetStoreManager plugin.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.NetStoreManager.Plugins.IStoragePlugin";
		return classes;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QObjectList Plugin::GetAccounts () const
	{
		return Util::Map (Accounts_, [] (const auto& acc) -> QObject* { return acc.get (); });
	}

	QIcon Plugin::GetStorageIcon () const
	{
		return QIcon (":/netstoremanager/dropbox/resources/images/dropbox-linux.png");
	}

	QString Plugin::GetStorageName () const
	{
		return "DropBox";
	}

	void Plugin::RegisterAccount (const QString& name)
	{
		Account *account = new Account (name, this);
		AuthManager_->Auth (account);
	}

	void Plugin::RemoveAccount (QObject *accObj)
	{
		auto pos = std::find_if (Accounts_.begin (), Accounts_.end (),
				[accObj] (const auto& acc) { return acc.get () == accObj; });
		if (pos == Accounts_.end ())
			return;

		emit accountRemoved (accObj);
		Accounts_.erase (pos);
		WriteAccounts ();
	}

	void Plugin::WriteAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_NSM_DB_Accounts");
		settings.beginWriteArray ("Accounts");
		for (int i = 0; i < Accounts_.size (); ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData", Accounts_.at (i)->Serialize ());
		}
		settings.endArray ();
	}

	void Plugin::ReadAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_NSM_DB_Accounts");
		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			const QByteArray& data = settings.value ("SerializedData").toByteArray ();
			Account_ptr acc = Account::Deserialize (data, this);
			Accounts_ << acc;
			emit accountAdded (acc.get ());
		}
		settings.endArray ();
	}

	void Plugin::handleAuthSuccess (QObject *accObj)
	{
		Account_ptr acc (qobject_cast<Account*> (accObj));
		Accounts_ << acc;
		WriteAccounts ();
		emit accountAdded (accObj);
	}

}
}
}


LC_EXPORT_PLUGIN (leechcraft_netstoremanager_dbox, LC::NetStoreManager::DBox::Plugin);
