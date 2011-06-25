/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "privacylistsconfigdialog.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QStandardItemModel>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	PrivacyListsConfigDialog::PrivacyListsConfigDialog (PrivacyListsManager *mgr, QWidget *parent)
	: QDialog (parent)
	, Manager_ (mgr)
	, Model_ (new QStandardItemModel)
	{
		Ui_.setupUi (this);
		Ui_.RulesTree_->setModel (Model_);
		
		QueryLists ();
	}
	
	void PrivacyListsConfigDialog::QueryLists ()
	{
		Ui_.StatusLabel_->setText (tr ("Fetching names of privacy lists..."));

		connect (Manager_,
				SIGNAL (gotLists (const QStringList&, const QString&, const QString&)),
				this,
				SLOT (handleGotLists (const QStringList&, const QString&, const QString&)));
		
		Manager_->QueryLists ();
	}
	
	void PrivacyListsConfigDialog::QueryList (const QString& list)
	{
		if (Lists_.contains (list))
		{
			handleGotList (Lists_ [list]);
			return;
		}

		Ui_.StatusLabel_->setText (tr ("Fetching list %1...").arg (list));
		
		connect (Manager_,
				SIGNAL (gotList (const PrivacyList&)),
				this,
				SLOT (handleGotList (const PrivacyList&)));
		
		Manager_->QueryList (list);
	}
	
	void PrivacyListsConfigDialog::AddListToBoxes (const QString& listName)
	{
		Ui_.ActiveList_->addItem (listName);
		Ui_.DefaultList_->addItem (listName);
		Ui_.ConfigureList_->addItem (listName);
	}
	
	QList<QStandardItem*> PrivacyListsConfigDialog::ToRow (const PrivacyListItem& item) const
	{
		QList<QStandardItem*> modelItems;

		switch (item.GetType ())
		{
		case PrivacyListItem::TNone:
			modelItems << new QStandardItem (tr ("None"));
			break;
		case PrivacyListItem::TJid:
			modelItems << new QStandardItem (tr ("JID"));
			break;
		case PrivacyListItem::TSubscription:
			modelItems << new QStandardItem (tr ("Subscription"));
			break;
		case PrivacyListItem::TGroup:
			modelItems << new QStandardItem (tr ("Group"));
			break;
		}
		
		modelItems << new QStandardItem (item.GetValue ());
		modelItems << new QStandardItem (item.GetAction () == PrivacyListItem::AAllow ?
					tr ("Allow") :
					tr ("Deny"));
		
		QStringList stanzasList;
		const PrivacyListItem::StanzaTypes types = item.GetStanzaTypes ();
		if (types == PrivacyListItem::STAll ||
				types == PrivacyListItem::STNone)
			stanzasList << tr ("All");
		else
		{
			if (types & PrivacyListItem::STMessage)
				stanzasList << tr ("Messages");
			if (types & PrivacyListItem::STPresenceIn)
				stanzasList << tr ("Incoming presences");
			if (types & PrivacyListItem::STPresenceOut)
				stanzasList << tr ("Outgoing presences");
			if (types & PrivacyListItem::STIq)
				stanzasList << tr ("IQ");
		}
		
		modelItems << new QStandardItem (stanzasList.join ("; "));

		return modelItems;
	}
	
	void PrivacyListsConfigDialog::accept ()
	{
		QDialog::accept ();
		
		Q_FOREACH (const PrivacyList& pl, Lists_.values ())
			Manager_->SetList (pl);
			
		Manager_->ActivateList (Ui_.ActiveList_->currentText (),
				PrivacyListsManager::LTActive);
		Manager_->ActivateList (Ui_.DefaultList_->currentText (),
				PrivacyListsManager::LTDefault);
		
		deleteLater ();
	}
	
	void PrivacyListsConfigDialog::reject ()
	{
		QDialog::reject ();
		
		deleteLater ();
	}
	
	void PrivacyListsConfigDialog::on_AddButton__released ()
	{
		const QString& listName = QInputDialog::getText (this,
				"LeechCraft",
				tr ("Please enter the name of the new list"));
		if (listName.isEmpty ())
			return;

		Lists_ [listName] = PrivacyList (listName);
		AddListToBoxes (listName);
		
		Ui_.ConfigureList_->setCurrentIndex (Ui_.ConfigureList_->findText (listName)); 
	}
	
	void PrivacyListsConfigDialog::on_RemoveButton__released ()
	{
		const QString& listName = Ui_.ConfigureList_->currentText ();
		if (listName.isEmpty ())
			return;
		
		if (Ui_.DefaultList_->currentText () == listName ||
				Ui_.ActiveList_->currentText () == listName)
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Unable to delete a list that is currently "
						"active or selected as default one."));
			return;
		}
		
		PrivacyList list (listName);
		Manager_->SetList (list);
		Lists_.remove (listName);
		Ui_.ConfigureList_->removeItem (Ui_.ConfigureList_->currentIndex ());
	}
	
	void PrivacyListsConfigDialog::handleGotLists (const QStringList& lists,
			const QString& active, const QString& def)
	{
		disconnect (Manager_,
				SIGNAL (gotLists (const QStringList&, const QString&, const QString&)),
				this,
				SLOT (handleGotLists (const QStringList&, const QString&, const QString&)));

		Ui_.ConfigureList_->clear ();
		Ui_.ConfigureList_->addItems (lists);
		Ui_.ActiveList_->clear ();
		Ui_.ActiveList_->addItems (QStringList (QString ()) + lists);
		Ui_.DefaultList_->clear ();
		Ui_.DefaultList_->addItems (QStringList (QString ()) + lists);
		
		int idx = Ui_.ActiveList_->findText (active);
		if (idx >= 0)
			Ui_.ActiveList_->setCurrentIndex (idx);
		idx = Ui_.DefaultList_->findText (def);
		if (idx >= 0)
			Ui_.DefaultList_->setCurrentIndex (idx);
		
		Ui_.StatusLabel_->setText (QString ());
		
		if (!lists.isEmpty ())
			QueryList (lists.at (0));
	}
	
	void PrivacyListsConfigDialog::handleGotList (const PrivacyList& list)
	{
		disconnect (Manager_,
				SIGNAL (gotList (const PrivacyList&)),
				this,
				SLOT (handleGotList (const PrivacyList&)));
		Ui_.StatusLabel_->setText (QString ());
		
		Model_->clear ();
		QStringList headers (tr ("Type"));
		headers << tr ("Value") << tr ("Action") << tr ("Stanzas");
		Model_->setHorizontalHeaderLabels (headers);
		
		Lists_ [list.GetName ()] = list;
		
		QList<PrivacyListItem> items = list.GetItems ();
		if (!items.isEmpty () && items.last ().GetType () == PrivacyListItem::TNone)
		{
			const PrivacyListItem& item = items.takeLast ();
			Ui_.DefaultPolicy_->setCurrentIndex (item.GetAction () == PrivacyListItem::AAllow ? 0 : 1);
		}
		
		Q_FOREACH (const PrivacyListItem& item, list.GetItems ())
			Model_->appendRow (ToRow (item));
	}
}
}
}
