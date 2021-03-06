/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "udevbackend.h"
#include <QSocketNotifier>
#include <QStandardItemModel>
#include <QtDebug>
#include <libudev.h>
#include <interfaces/devices/deviceroles.h>

namespace LC
{
namespace Devmon
{
	UDevBackend::UDevBackend (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, UDev_ (udev_new (), udev_unref)
	, Mon_ (udev_monitor_new_from_netlink (UDev_.get (), "udev"), udev_monitor_unref)
	, Model_ (new QStandardItemModel (this))
	{
		udev_set_userdata (UDev_.get (), this);

		udev_monitor_filter_add_match_subsystem_devtype (Mon_.get (), "usb", "usb_device");
		udev_monitor_enable_receiving (Mon_.get ());

		EnumerateAll ();

		Notifier_ = new QSocketNotifier (udev_monitor_get_fd (Mon_.get ()),
				QSocketNotifier::Read, this);
		connect (Notifier_,
				SIGNAL (activated (int)),
				this,
				SLOT (handleSocket (int)));
	}

	QAbstractItemModel* UDevBackend::GetModel () const
	{
		return Model_;
	}

	namespace
	{
		QHash<QString, QString> GetProperties (udev_device *device)
		{
			QHash<QString, QString> props;
			auto entry = udev_device_get_properties_list_entry (device);
			while (entry)
			{
				props [udev_list_entry_get_name (entry)] = udev_list_entry_get_value (entry);
				entry = udev_list_entry_get_next (entry);
			}
			return props;
		}

		QString GetID (const QHash<QString, QString>& props)
		{
			if (props.contains ("ID_SERIAL"))
				return props ["ID_SERIAL"];

			if (props.contains ("PRODUCT"))
				return props ["PRODUCT"];

			if (props.contains ("ID_VENDOR_ID") && props.contains ("ID_MODEL_ID"))
				return props ["ID_VENDOR_ID"] + ':' + props ["ID_MODEL_ID"];

			qWarning () << Q_FUNC_INFO
					<< "can't get ID for device"
					<< props;

			return "<unknown>";
		}

		void FillItem (QStandardItem *item, udev_device *device, const QHash<QString, QString>& props)
		{
			item->setText (props ["ID_VENDOR"] + " " + props ["ID_MODEL"]);
			item->setData (DeviceType::USBDevice, CommonDevRole::DevType);

			const auto& id = GetID (props);
			item->setData (id, CommonDevRole::DevID);
			item->setData (id, CommonDevRole::DevPersistentID);
			item->setData (props ["ID_VENDOR_ID"], USBDeviceRole::VendorID);
			item->setData (props ["ID_VENDOR"], USBDeviceRole::Vendor);
			item->setData (props ["ID_MODEL_ID"], USBDeviceRole::ModelID);
			item->setData (props ["ID_MODEL"], USBDeviceRole::Model);
			item->setData (props ["BUSNUM"].toInt (), USBDeviceRole::Busnum);
			item->setData (props ["DEVNUM"].toInt (), USBDeviceRole::Devnum);
			item->setData (QString (udev_device_get_syspath (device)), USBDeviceRole::SysFile);
		}
	}

	void UDevBackend::EnumerateAll ()
	{
		std::shared_ptr<udev_enumerate> enumerator (udev_enumerate_new (UDev_.get ()), udev_enumerate_unref);
		udev_enumerate_add_match_subsystem (enumerator.get (), "usb");
		udev_enumerate_add_match_is_initialized (enumerator.get ());

		udev_enumerate_scan_devices (enumerator.get ());
		auto entry = udev_enumerate_get_list_entry (enumerator.get ());
		while (entry)
		{
			std::shared_ptr<void> guard (nullptr,
					[&entry] (void*) { entry = udev_list_entry_get_next (entry); });

			const auto syspath = udev_list_entry_get_name (entry);
			std::shared_ptr<udev_device> device (udev_device_new_from_syspath (UDev_.get (), syspath), udev_device_unref);
			if (!device)
				continue;

			const auto type = udev_device_get_devtype (device.get ());
			if (!type || strcmp (type, "usb_device"))
				continue;

			const auto& props = GetProperties (device.get ());

			auto item = new QStandardItem ();
			FillItem (item, device.get (), props);
			Model_->appendRow (item);
		}
	}

	QStandardItem* UDevBackend::FindItemForPath (const QString& sysPath) const
	{
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			auto item = Model_->item (i);
			if (item->data (USBDeviceRole::SysFile).toString () == sysPath)
				return item;
		}

		return 0;
	}

	void UDevBackend::handleSocket (int)
	{
		Notifier_->setEnabled (false);
		std::shared_ptr<udev_device> device (udev_monitor_receive_device (Mon_.get ()), udev_device_unref);
		Notifier_->setEnabled (true);

		if (!device)
			return;

		const auto& props = GetProperties (device.get ());

		const QString action (udev_device_get_action (device.get ()));
		if (action == "add")
		{
			auto item = new QStandardItem ();
			FillItem (item, device.get (), props);
			Model_->appendRow (item);
		}
		else if (action == "remove")
		{
			auto item = FindItemForPath (udev_device_get_syspath (device.get ()));
			if (!item)
			{
				qWarning () << Q_FUNC_INFO
						<< "removed unknown item"
						<< udev_device_get_syspath (device.get ());
				return;
			}

			Model_->removeRow (item->row ());
		}
		else if (action == "change")
		{
			auto item = FindItemForPath (udev_device_get_syspath (device.get ()));
			if (!item)
			{
				qWarning () << Q_FUNC_INFO
						<< "changed unknown item"
						<< udev_device_get_syspath (device.get ());
				return;
			}
			FillItem (item, device.get (), props);
		}
	}
}
}
