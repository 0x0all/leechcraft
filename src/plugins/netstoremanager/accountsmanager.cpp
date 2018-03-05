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

#include "accountsmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "interfaces/netstoremanager/istorageplugin.h"
#include "interfaces/netstoremanager/istorageaccount.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	AccountsManager::AccountsManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	, Proxy_ (proxy)
	{
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Account")) << tr ("Storage"));
	}

	void AccountsManager::AddPlugin (IStoragePlugin *plug)
	{
		Plugins_ << plug;

		Q_FOREACH (QObject *acc, plug->GetAccounts ())
			handleAccountAdded (acc);

		connect (plug->GetQObject (),
				SIGNAL (accountAdded (QObject*)),
				this,
				SLOT (handleAccountAdded (QObject*)));
		connect (plug->GetQObject (),
				SIGNAL (accountRemoved (QObject*)),
				this,
				SLOT (handleAccountRemoved (QObject*)));
	}

	QList<IStoragePlugin*> AccountsManager::GetPlugins () const
	{
		return Plugins_;
	}

	QList<IStorageAccount*> AccountsManager::GetAccounts () const
	{
		QList<IStorageAccount*> accounts;
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			auto data = Model_->item (i)->data (Roles::AccountObj);
			accounts << qobject_cast<IStorageAccount*> (data.value<QObject*> ());
		}
		return accounts;
	}

	IStorageAccount* AccountsManager::GetAccountFromUniqueID (const QString& id) const
	{
		auto accounts = GetAccounts ();
		auto it = std::find_if (accounts.begin (), accounts.end (),
				[id] (const auto& acc) { return acc->GetUniqueID () == id; });
		return it == accounts.end () ? 0 : *it;
	}

	QAbstractItemModel* AccountsManager::GetModel () const
	{
		return Model_;
	}

	void AccountsManager::RemoveAccount (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		QObject *accObj = index.sibling (index.row (), 0)
				.data (Roles::AccountObj).value<QObject*> ();
		auto acc = qobject_cast<IStorageAccount*> (accObj);

		auto plugin = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
		plugin->RemoveAccount (accObj);
	}

	ICoreProxy_ptr AccountsManager::GetProxy () const
	{
		return Proxy_;
	}

	void AccountsManager::handleAccountAdded (QObject *obj)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (obj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj;
			return;
		}

		QObject *pObj = acc->GetParentPlugin ();
		IStoragePlugin *plugin = qobject_cast<IStoragePlugin*> (pObj);

		QList<QStandardItem*> row;
		row << new QStandardItem (plugin->GetStorageIcon (), acc->GetAccountName ());
		row << new QStandardItem (plugin->GetStorageName ());
		Model_->appendRow (row);

		row.first ()->setData (QVariant::fromValue<QObject*> (obj), Roles::AccountObj);
		emit accountAdded (obj);
	}

	void AccountsManager::handleAccountRemoved (QObject *obj)
	{
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			if (Model_->item (i)->data (AccountObj).value<QObject*> () != obj)
				continue;

			Model_->removeRow (i);
			break;
		}

		emit accountRemoved (obj);
	}
}
}
