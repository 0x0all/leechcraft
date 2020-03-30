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

#include "geoip.h"
#include <QStringList>
#include <QFile>
#include <QtDebug>
#include <util/sll/monad.h>

#ifdef ENABLE_GEOIP
#include <maxminddb.h>
#endif

namespace LC::BitTorrent
{
#ifdef ENABLE_GEOIP
	namespace
	{
		std::optional<QString> FindDB ()
		{
			const QStringList geoipCands
			{
				"/usr/share/GeoIP",
				"/usr/local/share/GeoIP",
				"/var/lib/GeoIP"
			};

			for (const auto& cand : geoipCands)
			{
				const auto& name = cand + "/GeoLite2-Country.mmdb";
				if (QFile::exists (name))
					return { name };
			}

			return {};
		}
	}

	GeoIP::GeoIP ()
	{
		using Util::operator>>;

		const auto maybeImpl = FindDB () >>
				[] (const QString& path) -> std::optional<ImplPtr_t>
				{
					qDebug () << Q_FUNC_INFO << "loading GeoIP from" << path;

					MMDB_s mmdb;
					if (int status = MMDB_open (path.toStdString ().c_str (), MMDB_MODE_MMAP, &mmdb);
						status != MMDB_SUCCESS)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to load MaxMind DB from:"
								<< status;
						return {};
					}

					auto ptr = ImplPtr_t { new MMDB_s, &MMDB_close };
					*ptr = mmdb;
					return { ptr };
				};
		Impl_ = maybeImpl.value_or (ImplPtr_t {});
	}

	std::optional<QString> GeoIP::GetCountry (const libtorrent::address& addr) const
	{
		if (!Impl_)
			return {};

		int gai_error;
		int mmdb_error;
		auto entry = MMDB_lookup_string (Impl_.get (), addr.to_string ().c_str (), &gai_error, &mmdb_error);
		if (gai_error != 0 || mmdb_error != MMDB_SUCCESS)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to query MMDB for"
					<< addr.to_string ().c_str ();
			return {};
		}

		if (!entry.found_entry)
			return {};

		MMDB_entry_data_s entryData;
		if (int result = MMDB_get_value (&entry.entry, &entryData, "country", "iso_code", NULL);
			result != MMDB_SUCCESS || !entryData.has_data || !entryData.utf8_string)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to query MMDB entry for the country iso code:"
					<< result
					<< entryData.has_data;
			return {};
		}

		return QString::fromLatin1 (entryData.utf8_string, 2).toLower ();
	}
#else
	GeoIP::GeoIP ()
	{
	}

	boost::optional<QString> GeoIP::GetCountry (const libtorrent::address&) const
	{
		return {};
	}
#endif
}
