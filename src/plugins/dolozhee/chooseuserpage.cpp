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

#include "chooseuserpage.h"
#include <QSettings>
#include <QtDebug>
#include <util/xpc/passutils.h>
#include "reportwizard.h"

namespace LC
{
namespace Dolozhee
{
	ChooseUserPage::ChooseUserPage (const ICoreProxy_ptr& proxy, QWidget *parent)
	: QWizardPage (parent)
	, Proxy_ (proxy)
	{
		Ui_.setupUi (this);

		for (const auto but : findChildren<QRadioButton*> ())
			connect (but,
					&QRadioButton::toggled,
					this,
					&ChooseUserPage::completeChanged);
		for (const auto edit : findChildren<QLineEdit*> ())
			connect (edit,
					&QLineEdit::textChanged,
					this,
					&ChooseUserPage::completeChanged);
	}

	void ChooseUserPage::initializePage ()
	{
		connect (wizard (),
				&QDialog::accepted,
				this,
				&ChooseUserPage::SaveCredentials);

		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Dolozhee");
		settings.beginGroup ("Credentials");
		const QString& login = settings.value ("Login").toString ();
		settings.endGroup ();

		if (login.isEmpty ())
			return;

		Ui_.Existing_->setChecked (true);

		Ui_.Login_->setText (login);

		const QString& text = tr ("Please enter password for user %1:")
				.arg (login);
		const QString& pass = Util::GetPassword (GetPassKey (), text, Proxy_);
		Ui_.Password_->setText (pass);
	}

	int ChooseUserPage::nextId () const
	{
		return GetUser () != User::New ?
				ReportWizard::PageID::ReportType :
				ReportWizard::PageID::UserStatus;
	}

	bool ChooseUserPage::isComplete () const
	{
		switch (GetUser ())
		{
		case User::Anonymous:
			return true;
		case User::Existing:
			return !GetLogin ().isEmpty () && !GetPassword ().isEmpty ();
		default:
			return false;
		}
	}

	ChooseUserPage::User ChooseUserPage::GetUser () const
	{
		if (Ui_.New_->isChecked ())
			return User::New;
		else if (Ui_.Existing_->isChecked ())
			return User::Existing;
		else
			return User::Anonymous;
	}

	QString ChooseUserPage::GetLogin () const
	{
		return GetUser () == User::Anonymous ?
				"7b034124da8534c8e3464afd4dd59abc244bd271" :
				Ui_.Login_->text ();
	}

	QString ChooseUserPage::GetPassword () const
	{
		return GetUser () == User::Anonymous ?
				"somepass" :
				Ui_.Password_->text ();
	}

	QString ChooseUserPage::GetEmail () const
	{
		return Ui_.EMail_->text ();
	}

	QString ChooseUserPage::GetFirstName () const
	{
		return Ui_.FirstName_->text ();
	}

	QString ChooseUserPage::GetLastName () const
	{
		return Ui_.LastName_->text ();
	}

	QString ChooseUserPage::GetPassKey () const
	{
		return "org.LeechCraft.Dolozhee.Username_" + Ui_.Login_->text ();
	}

	void ChooseUserPage::SaveCredentials ()
	{
		if (GetUser () != User::Existing)
			return;

		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Dolozhee");
		settings.beginGroup ("Credentials");
		settings.setValue ("Login", GetLogin ());
		settings.endGroup ();

		Util::SavePassword (GetPassword (), GetPassKey (), Proxy_);
	}
}
}
