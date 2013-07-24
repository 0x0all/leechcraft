/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "unmountabledevmanager.h"
#include <QStandardItemModel>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "../interfaces/lmp/iunmountablesync.h"
#include "../core.h"

namespace LeechCraft
{
namespace LMP
{
	UnmountableDevManager::UnmountableDevManager (QObject *parent)
	: QObject (parent)
	, DevListModel_ (new QStandardItemModel (this))
	{
		DevListModel_->setColumnCount (1);
	}

	QAbstractItemModel* UnmountableDevManager::GetDevListModel () const
	{
		return DevListModel_;
	}

	void UnmountableDevManager::InitializePlugins ()
	{
		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		Managers_ = pm->GetAllCastableRoots<IUnmountableSync*> ();
		Q_FOREACH (auto mgr, Managers_)
			connect (mgr,
					SIGNAL (availableDevicesChanged ()),
					this,
					SLOT (rebuildAvailableDevices ()));

		rebuildAvailableDevices ();
	}

	QObject* UnmountableDevManager::GetDeviceManager (int row) const
	{
		auto item = DevListModel_->item (row);
		if (!item)
			return 0;

		return item->data (Roles::ManagerObj).value<QObject*> ();
	}

	UnmountableDevInfo UnmountableDevManager::GetDeviceInfo (int row) const
	{
		auto item = DevListModel_->item (row);
		if (!item)
			return UnmountableDevInfo ();

		return item->data (Roles::DeviceInfo).value<UnmountableDevInfo> ();
	}

	void UnmountableDevManager::rebuildAvailableDevices ()
	{
		if (const auto rc = DevListModel_->rowCount ())
			DevListModel_->removeRows (0, rc);

		Q_FOREACH (auto mgrObj, Managers_)
		{
			auto mgr = qobject_cast<IUnmountableSync*> (mgrObj);
			for (const auto& device : mgr->AvailableDevices ())
			{
				auto item = new QStandardItem (device.Name_);
				item->setData (QVariant::fromValue (mgrObj), Roles::ManagerObj);
				item->setData (device.ID_, CommonDevRole::DevPersistentID);
				item->setData (QVariant::fromValue (device), Roles::DeviceInfo);
				DevListModel_->appendRow (item);
			}
		}
	}
}
}
