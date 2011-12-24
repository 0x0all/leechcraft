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

#include "manageblacklistdialog.h"
#include <QStandardItemModel>
#include "msnaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	ManageBlackListDialog::ManageBlackListDialog (MSNAccount *acc, QWidget *parent)
	: QDialog (parent)
	, Account_ (acc)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Q_FOREACH (const QString& str, acc->GetBL ())
			Model_->appendRow (new QStandardItem (str));

		Ui_.BL_->setModel (Model_);
	}

	void ManageBlackListDialog::on_Remove__released ()
	{
		QStandardItem *item = Model_->itemFromIndex (Ui_.BL_->currentIndex ());
		if (!item)
			return;

		Account_->RemoveFromBL (item->text ());
		Model_->removeRow (item->row ());
	}
}
}
}
