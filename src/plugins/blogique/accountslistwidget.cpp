/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "accountslistwidget.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include <QtDebug>
#include <QWizard>
#include <stdexcept>
#include "interfaces/blogique/ibloggingplatform.h"
#include "interfaces/blogique/iaccount.h"
#include "addaccountwizardfirstpage.h"
#include "core.h"
#include "profiledialog.h"
#include "storagemanager.h"

namespace LC
{
namespace Blogique
{
	AccountsListWidget::AccountsListWidget (QWidget *parent)
	: QWidget (parent)
	, AccountsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		connect (&Core::Instance (),
				SIGNAL (accountAdded (QObject*)),
				this,
				SLOT (addAccount (QObject*)));
		connect (&Core::Instance (),
				SIGNAL (accountRemoved (QObject*)),
				this,
				SLOT (handleAccountRemoved (QObject*)));
		connect (&Core::Instance (),
				SIGNAL (accountValidated (QObject*, bool)),
				this,
				SLOT (handleAccountValidated (QObject*, bool)));

		connect (Ui_.Accounts_,
				SIGNAL (clicked (const QModelIndex&)),
				this,
				SLOT (handleAccountClicked (const QModelIndex&)));

		connect (Ui_.Accounts_,
				SIGNAL (doubleClicked (const QModelIndex&)),
				this,
				SLOT (handleAccountDoubleClicked (const QModelIndex&)));

		for (const auto acc : Core::Instance ().GetAccounts ())
			addAccount (acc->GetQObject ());

		AccountsModel_->setHorizontalHeaderLabels ({tr ("Account"), tr ("Validated")});
		Ui_.Accounts_->setModel (AccountsModel_);

		Ui_.Profile_->setEnabled (false);
	}

	void AccountsListWidget::addAccount (QObject *accObj)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< accObj
					<< "is not an IAccount";
			return;
		}

		IBloggingPlatform *ibp = qobject_cast<IBloggingPlatform*> (acc->
				GetParentBloggingPlatform ());

		QStandardItem *item = new QStandardItem (acc->GetAccountName ());
		item->setIcon (ibp ? ibp->GetBloggingPlatformIcon () : QIcon ());
		item->setEditable (false);
		QStandardItem *itemValidated = new QStandardItem (acc->IsValid () ?
				tr ("Validated") :
				tr ("Not validated"));
		itemValidated->setEditable (false);
		AccountsModel_->appendRow ({item, itemValidated});

		Ui_.Accounts_->header ()->setSectionResizeMode (QHeaderView::ResizeToContents);

		Item2Account_ [item] = acc;
		Account2Item_ [acc] = item;

		try
		{
			Core::Instance ().GetStorageManager ()->AddAccount (acc->GetAccountID ());
		}
		catch (const std::runtime_error& e)
		{
			QMessageBox::warning (this,
					tr ("LeechCraft"),
					tr ("Error adding account."));
		}
	}

	void AccountsListWidget::handleAccountRemoved (QObject *accObj)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< accObj
					<< "is not an IAccount";
			return;
		}

		if (!Account2Item_.contains (acc))
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< acc->GetAccountName ()
					<< acc->GetQObject ()
					<< "from"
					<< sender ()
					<< "not found here";
			return;
		}

		QStandardItem *item = Account2Item_ [acc];
		Item2Account_.remove (item);
		AccountsModel_->removeRow (item->row ());
		Account2Item_.remove (acc);
		try
		{
			Core::Instance ().GetStorageManager ()->RemoveAccount (acc->GetAccountID ());
		}
		catch (const std::runtime_error& e)
		{
			QMessageBox::warning (this,
					tr ("LeechCraft"),
					tr ("Error removing account."));
		}
	}

	void AccountsListWidget::handleAccountValidated (QObject *accObj, bool validated)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< accObj
					<< "is not an IAccount";
			return;
		}

		if (!Account2Item_.contains (acc))
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< acc->GetAccountName ()
					<< acc->GetQObject ()
					<< "from"
					<< sender ()
					<< "not found here";
			return;
		}

		QStandardItem *item = Account2Item_ [acc];
		AccountsModel_->item (item->row (), Columns::IsValidated)->setText (validated ?
				tr ("Validated") :
				tr ("Not validated"));
		Ui_.Accounts_->header ()->setSectionResizeMode (QHeaderView::ResizeToContents);
	}

	void AccountsListWidget::on_Add__released ()
	{
		QWizard *wizard = new QWizard (this);
		wizard->setAttribute (Qt::WA_DeleteOnClose);
		wizard->setWindowTitle (tr ("Add account"));
		wizard->addPage (new AddAccountWizardFirstPage (wizard));

		wizard->show ();
	}

	void AccountsListWidget::on_Modify__released ()
	{
		auto index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), Columns::Name);
		if (!index.isValid ())
			return;

		QStandardItem *item = AccountsModel_->itemFromIndex (index);
		if (item &&
				Item2Account_.contains (item))
			Item2Account_ [item]->OpenConfigurationDialog ();
	}

	void AccountsListWidget::on_Delete__released ()
	{
		auto index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), Columns::Name);
		if (!index.isValid ())
			return;

		QStandardItem *item = AccountsModel_->itemFromIndex (index);
		IAccount *acc = 0;
		if (item &&
				Item2Account_.contains (item))
			acc = Item2Account_ [item];
		else
			return;

		if (QMessageBox::question (this,
				"LeechCraft",
				tr ("Are you sure you want to remove the account %1?")
						.arg ("<em>" + acc->GetAccountName () + "</em>"),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		QObject *bpObj = acc->GetParentBloggingPlatform ();
		IBloggingPlatform *ibp = qobject_cast<IBloggingPlatform*> (bpObj);
		if (!ibp)
		{
			qWarning () << Q_FUNC_INFO
					<< "parent blogging platform for"
					<< acc->GetAccountID ()
					<< "doesn't implement IBloggingPlatform";
			return;
		}
		ibp->RemoveAccount (acc->GetQObject ());
	}

	void AccountsListWidget::on_Profile__released ()
	{
		auto index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), Columns::Name);
		if (!index.isValid ())
			return;

		QStandardItem *item = AccountsModel_->itemFromIndex (index);
		if (item &&
				Item2Account_.contains (item))
		{
			ProfileDialog *pd = new ProfileDialog (Item2Account_ [item], this);
			pd->setAttribute (Qt::WA_DeleteOnClose);
			pd->show ();
		}
	}

	void AccountsListWidget::handleAccountClicked (const QModelIndex& idx)
	{
		QModelIndex index = idx.sibling (idx.row (), Columns::Name);
		if (!index.isValid ())
			return;

		QStandardItem *item = AccountsModel_->itemFromIndex (index);
		if (item &&
				Item2Account_.contains (item))
		{
			IAccount *acc = Item2Account_ [item];
			auto ibp = qobject_cast<IBloggingPlatform*> (acc->GetParentBloggingPlatform ());
			if (!ibp)
			{
				qWarning () << Q_FUNC_INFO
						<< "account"
						<< acc->GetAccountID ()
						<< "hasn't valid parent blogging platform"
						<< acc->GetParentBloggingPlatform ();
				return;
			}

			Ui_.Profile_->setEnabled ((ibp->GetFeatures () &
					IBloggingPlatform::BPFSupportsProfiles) &&
					acc->IsValid ());
		}
	}

	void AccountsListWidget::handleAccountDoubleClicked (const QModelIndex&)
	{
		on_Profile__released ();
	}

}
}
