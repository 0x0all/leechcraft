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

#include "firefoximportpage.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QtDebug>

namespace LC
{
namespace NewLife
{
namespace Importers
{
	FirefoxImportPage::FirefoxImportPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);

		Ui_.ImportSettings_->setText (Ui_.ImportSettings_->text ().arg ("Firefox"));

		setTitle (tr ("Firefox's data import"));
		setSubTitle (tr ("Select Firefox's INI file"));
		registerField ("ProfileFile", Ui_.FileLocation_);
	}

	bool FirefoxImportPage::CheckValidity (const QString& filename) const
	{
		QFile file (filename);
		if (!file.exists () ||
				!file.open (QIODevice::ReadOnly))
			return false;
		return true;
	}

	bool FirefoxImportPage::isComplete () const
	{
		return CheckValidity (Ui_.FileLocation_->text ());
	}

	void FirefoxImportPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (currentIdChanged (int)),
				this,
				SLOT (handleAccepted (int)));
#ifdef Q_OS_WIN32
		QString defaultFile = QDir::homePath () + "/Application Data/Mozilla/Firefox/profiles.ini";
#elif defined Q_OS_MAC
#warning Please check location of stuff on Mac OS X
		QString defaultFile = QDir::homePath () + "/Library/Application Support/Firefox/profiles.ini";
#else
		QString defaultFile = QDir::homePath () + "/.mozilla/firefox/profiles.ini";
#endif
		if (CheckValidity (defaultFile))
			Ui_.FileLocation_->setText (defaultFile);
	}

	void FirefoxImportPage::on_Browse__released ()
	{
		QString filename = QFileDialog::getOpenFileName (this,
				tr ("Select Firefox's INI file"),
				QDir::homePath () + "/.mozilla/",
				tr ("INI files (*.ini);;All files (*.*)"));
		if (filename.isEmpty ())
			return;

		if (!CheckValidity (filename))
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("The file you've selected is not a valid INI file."));
		else
			Ui_.FileLocation_->setText (filename);

		emit completeChanged ();
	}

	void FirefoxImportPage::on_FileLocation__textEdited (const QString&)
	{
		emit completeChanged ();
	}

	void FirefoxImportPage::handleAccepted (int)
	{
		setField ("ProfileFile", Ui_.FileLocation_->text ());
	}
}
}
}
