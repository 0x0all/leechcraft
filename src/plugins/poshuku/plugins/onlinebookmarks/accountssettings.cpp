/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountssettings.h"
#include <QStandardItemModel>
#include <QDateTime>
#include <QTimer>
#include <QLayout>
#include <util/sll/qtutil.h>
#include <interfaces/iauthwidget.h>
#include <interfaces/iaccount.h>
#include "core.h"

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	AccountsSettings::AccountsSettings ()
	: AccountsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Ui_.AccountsView_->setModel (AccountsModel_);

		AccountsModel_->setHorizontalHeaderLabels (QStringList () << tr ("Account")
				<< tr ("Last upload date")
				<< tr ("Last download date"));
		Ui_.Delete_->setEnabled (false);
		Ui_.Register_->hide ();
		Ui_.LoginFrame_->hide ();
	}

	AccountsSettings::~AccountsSettings ()
	{
		qDeleteAll (Service2AuthWidget_);
	}

	void AccountsSettings::InitServices ()
	{
		for (auto plugin : Core::Instance ().GetServicePlugins ())
		{
			IBookmarksService *ibs = qobject_cast<IBookmarksService*> (plugin);
			QAction *act = new QAction (ibs->GetServiceIcon (), ibs->GetServiceName (), this);
			Action2Service_ [act] = ibs;
			Ui_.AddAccount_->addAction (act);

			QWidget *widget = ibs->GetAuthWidget ();
			if (!qobject_cast<IAuthWidget*> (widget))
			{
				qWarning () << Q_FUNC_INFO
						<< "auth widget for plugin"
						<< plugin
						<< "is not a IAuthWidget"
						<< widget;
				continue;
			}

			Service2AuthWidget_ [ibs] = widget;

			connect (ibs->GetQObject (),
					SIGNAL (accountAdded (QObjectList)),
					this,
					SLOT (addAccount (QObjectList)));

			connect (this,
					SIGNAL (accountRemoved (QObject*)),
					ibs->GetQObject (),
					SLOT (removeAccount (QObject*)));
		}
	}

	QStandardItemModel* AccountsSettings::GetAccountsModel () const
	{
		return AccountsModel_;
	}

	void AccountsSettings::UpdateDates ()
	{
		QLocale loc;
		for (auto [item, acc] : Util::Stlize (Item2Account_))
		{
			int row = item->row ();
			const auto parentItem = item->parent ();
			parentItem->child (row, 1)->setText (loc.toString (acc->GetLastUploadDateTime (), QLocale::ShortFormat));
			parentItem->child (row, 2)->setText (loc.toString (acc->GetLastDownloadDateTime (), QLocale::ShortFormat));
		}
	}

	QModelIndex AccountsSettings::GetServiceIndex (QObject *serviceObj) const
	{
		for (auto [item, service] : Util::Stlize (Item2Service_))
			if (service == qobject_cast<IBookmarksService*> (serviceObj))
				return item->index ();

		return {};
	}

	void AccountsSettings::ScheduleResize ()
	{
		if (Scheduled_)
			return;

		QTimer::singleShot (100, this, SLOT (resizeColumns ()));
		Scheduled_ = true;
	}

	void AccountsSettings::accept ()
	{
		QObjectList accounts;
		for (auto [item, acc] : Util::Stlize (Item2Account_))
			if (item->checkState () == Qt::Checked)
			{
				acc->SetSyncing (true);
				accounts << acc->GetQObject ();
			}

		for (auto service : Item2Service_)
			service->saveAccounts ();

		Core::Instance ().SetActiveAccounts (accounts);
		Core::Instance ().checkDownloadPeriod ();
		Core::Instance ().checkUploadPeriod ();
	}

	void AccountsSettings::resizeColumns ()
	{
		for (int i = 0; i < 3; ++i)
			Ui_.AccountsView_->resizeColumnToContents (i);
	}

	void AccountsSettings::on_Delete__clicked ()
	{
		const QModelIndex& current = Ui_.AccountsView_->currentIndex ();
		const int row = current.row ();
		const QModelIndex& parentIndex = current.parent ();
		if (parentIndex == QModelIndex ())
			return;

		QStandardItem *item = AccountsModel_->itemFromIndex (parentIndex)->child (row, 0);

		Core::Instance ().DeletePassword (Item2Account_ [item]->GetQObject ());
		emit accountRemoved (Item2Account_ [item]->GetQObject ());

		AccountsModel_->removeRow (current.row (), parentIndex);
		Id2Account_.remove (Item2Account_ [item]->GetAccountID ());
		Item2Account_.remove (item);

		if (!AccountsModel_->rowCount (parentIndex))
		{
			Item2Service_.remove (AccountsModel_->itemFromIndex (parentIndex));
			AccountsModel_->removeRow (parentIndex.row ());
		}
	}

	void AccountsSettings::on_Auth__clicked ()
	{
		IBookmarksService *ibs = Service2AuthWidget_.key (LastWidget_);
		if (!ibs)
			return;

		IAuthWidget *aw = qobject_cast<IAuthWidget*> (LastWidget_);
		if (!aw)
		{
			qWarning () << Q_FUNC_INFO
					<< "is not a IAuthWidget"
					<< LastWidget_;
			return;
		}

		ibs->CheckAuthData (aw->GetIdentifyingData ());
	}

	void AccountsSettings::on_Register__clicked ()
	{
		IBookmarksService *ibs = Service2AuthWidget_.key (LastWidget_);
		if (!ibs)
			return;

		IAuthWidget *aw = qobject_cast<IAuthWidget*> (LastWidget_);
		if (!aw)
		{
			qWarning () << Q_FUNC_INFO
			<< "is not a IAuthWidget"
			<< LastWidget_;
			return;
		}

		ibs->RegisterAccount (aw->GetIdentifyingData ());
	}

	void AccountsSettings::on_AccountsView__clicked (const QModelIndex& index)
	{
		Ui_.Delete_->setEnabled (index.parent().isValid ());
	}

	void AccountsSettings::on_AddAccount__triggered (QAction *action)
	{
		if (!Action2Service_.contains (action))
			return;

		if (LastWidget_)
		{
			Ui_.AuthWidget_->layout ()->removeWidget (LastWidget_);
			LastWidget_->hide ();
		}

		IBookmarksService *ibs = Action2Service_ [action];

		Ui_.Register_->setVisible (ibs->GetFeatures () &
			IBookmarksService::FCanRegisterAccount);

		if (!Service2AuthWidget_.contains (ibs))
			return;

		IAuthWidget *aw = qobject_cast<IAuthWidget*> (Service2AuthWidget_ [ibs]);
		aw->SetIdentifyingData (QVariantMap ());

		Ui_.AuthWidget_->layout ()->addWidget (Service2AuthWidget_ [ibs]);
		Service2AuthWidget_ [ibs]->show ();
		Ui_.ControlLayout_->insertWidget (1, Ui_.LoginFrame_);
		Ui_.LoginFrame_->show ();
		LastWidget_ = Service2AuthWidget_ [ibs];
	}

	void AccountsSettings::on_Close__clicked ()
	{
		Ui_.AuthWidget_->layout ()->removeWidget (LastWidget_);
		LastWidget_->hide ();
		Ui_.ControlLayout_->removeWidget (Ui_.LoginFrame_);
		Ui_.LoginFrame_->hide ();
	}

	void AccountsSettings::addAccount (QObjectList accObjects)
	{
		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (sender ());
		if (!ibs)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "ins't IBookmarksService";
			return;
		}

		for (auto accObj : accObjects)
		{
			IAccount *account = qobject_cast<IAccount*> (accObj);
			if (Id2Account_.contains (account->GetAccountID ()))
				continue;

			if (!account->GetPassword ().isEmpty ())
				Core::Instance ().SavePassword (accObj);
			else
				account->SetPassword (Core::Instance ().GetPassword (accObj));

			Id2Account_ [account->GetAccountID ()] = accObj;

			const QModelIndex& index = GetServiceIndex (ibs->GetQObject ());
			QStandardItem *parentItem = 0;
			if (!index.isValid ())
			{
				parentItem = new QStandardItem (ibs->GetServiceIcon (), ibs->GetServiceName ());
				parentItem->setEditable (false);
				Item2Service_ [parentItem] = ibs;
				AccountsModel_->appendRow (parentItem);
			}
			else
				parentItem = AccountsModel_->itemFromIndex (index);

			auto item = new QStandardItem (account->GetLogin ());
			item->setCheckable (true);
			item->setCheckState (account->IsSyncing () ? Qt::Checked : Qt::Unchecked);

			if (account->IsSyncing ())
			{
				Core::Instance ().AddActiveAccount (accObj);
				IBookmarksService *ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
				ibs->DownloadBookmarks (account->GetQObject (), account->GetLastDownloadDateTime ());
				ibs->UploadBookmarks (account->GetQObject (), Core::Instance ().GetAllBookmarks ());
			}

			Item2Account_ [item] = account;

			QLocale loc;
			auto uploaditem = new QStandardItem (loc.toString (account->GetLastUploadDateTime (), QLocale::ShortFormat));
			auto downloaditem = new QStandardItem (loc.toString (account->GetLastDownloadDateTime (), QLocale::ShortFormat));

			QList<QStandardItem*> record { item, uploaditem, downloaditem };
			for (auto cell : record)
				cell->setEditable (false);
			parentItem->appendRow (record);

			if (account->IsSyncing ())
				Core::Instance ().AddActiveAccount (accObj);

			Ui_.AccountsView_->expandAll ();

			Scheduled_ = false;
			ScheduleResize ();
		}
	}

}
}
}
