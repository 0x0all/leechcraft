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

#include "fotobilderservice.h"
#include <QIcon>
#include <QCoreApplication>
#include <QSettings>
#include <QtDebug>
#include <util/xpc/passutils.h>
#include "fotobilderaccount.h"
#include "registerpage.h"

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	FotoBilderService::FotoBilderService (ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	{
		ReadAccounts ();
	}

	QObject* FotoBilderService::GetQObject ()
	{
		return this;
	}

	QString FotoBilderService::GetServiceName () const
	{
		return "LJ FotoBilder";
	}

	QIcon FotoBilderService::GetServiceIcon () const
	{
		static QIcon icon (":/blasq/deathnote/resources/images/livejournalicon.svg");
		return icon;
	}

	QList<IAccount*> FotoBilderService::GetRegisteredAccounts () const
	{
		QList<IAccount*> result;
		for (auto acc : Accounts_)
			result << acc;
		return result;
	}

	QList<QWidget*> FotoBilderService::GetAccountRegistrationWidgets () const
	{
		return { new RegisterPage };
	}

	void FotoBilderService::RegisterAccount (const QString& name,
			const QList<QWidget*>& widgets)
	{
		if (!widgets.isEmpty ())
		{
			auto rp = qobject_cast<RegisterPage*> (widgets.at (0));
			const auto& login = rp->GetLogin ();
			//TODO error
			if (login.isEmpty ())
				return;

			const auto& password = rp->GetPassword ();
			auto acc = new FotoBilderAccount (name, this, Proxy_, login);

			if (!password.isEmpty ())
				Util::SavePassword (password,
						"org.LeechCraft.Blasq.PassForAccount/" + acc->GetID (),
						Proxy_);
			AddAccount (acc);
			saveAccount (acc);
		}
	}

	void FotoBilderService::RemoveAccount (IAccount *account)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_DeathNote");
		settings.beginGroup ("Accounts");
		settings.remove (account->GetID ());
		settings.endGroup ();

		const auto pos = std::find (Accounts_.begin (), Accounts_.end (), account);
		if (pos == Accounts_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< account->GetID ()
					<< "not found";
			return;
		}

		emit accountRemoved (*pos);
		(*pos)->deleteLater ();

		Accounts_.erase (pos);
	}

	void FotoBilderService::AddAccount (FotoBilderAccount *account)
	{
		Accounts_ << account;
		emit accountAdded (account);
		connect (account,
				SIGNAL (accountChanged (FotoBilderAccount*)),
				this,
				SLOT (saveAccount (FotoBilderAccount*)));
	}

	void FotoBilderService::ReadAccounts ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_DeathNote");
		settings.beginGroup ("Accounts");
		for (const auto& key : settings.childKeys ())
		{
			const auto& serialized = settings.value (key).toByteArray ();
			if (auto acc = FotoBilderAccount::Deserialize (serialized, this, Proxy_))
				AddAccount (acc);
		}
		settings.endGroup ();
	}

	void FotoBilderService::saveAccount (FotoBilderAccount *account)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_DeathNote");
		settings.beginGroup ("Accounts");
		settings.setValue (account->GetID (), account->Serialize ());
		settings.endGroup ();
	}
}
}
}
