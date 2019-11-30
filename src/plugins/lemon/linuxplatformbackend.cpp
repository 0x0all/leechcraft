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

#include "linuxplatformbackend.h"
#include <QStringList>
#include <QtDebug>
#include <netlink/route/addr.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>
#include <netlink/route/route.h>

namespace LC
{
namespace Lemon
{
	LinuxPlatformBackend::LinuxPlatformBackend (QObject *parent)
	: PlatformBackend (parent)
	, Rtsock_ (nl_socket_alloc ())
	{
		if (nl_connect (Rtsock_, NETLINK_ROUTE) >= 0)
			rtnl_link_alloc_cache (Rtsock_, AF_UNSPEC, &LinkCache_);
		else
			qWarning () << Q_FUNC_INFO
					<< "unable to establish netlink conn";
	}

	LinuxPlatformBackend::~LinuxPlatformBackend ()
	{
		nl_cache_free (LinkCache_);
		nl_close (Rtsock_);
		nl_socket_free (Rtsock_);
	}

	auto LinuxPlatformBackend::GetCurrentNumBytes (const QString& name) const -> CurrentTrafficState
	{
		return DevInfos_ [name].Traffic_;
	}

	void LinuxPlatformBackend::update (const QStringList& devices)
	{
		if (!LinkCache_)
			return;

		nl_cache_refill (Rtsock_, LinkCache_);

		for (const auto& devName : devices)
		{
			auto link = rtnl_link_get_by_name (LinkCache_, devName.toLocal8Bit ().constData ());
			if (!link)
			{
				qWarning () << Q_FUNC_INFO
						<< "no link for device"
						<< devName;
				continue;
			}

			auto& info = DevInfos_ [devName];

			info.Traffic_.Down_ = rtnl_link_get_stat (link, RTNL_LINK_RX_BYTES);
			info.Traffic_.Up_ = rtnl_link_get_stat (link, RTNL_LINK_TX_BYTES);

			rtnl_link_put (link);
		}
	}
}
}
