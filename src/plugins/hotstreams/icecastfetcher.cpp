/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "icecastfetcher.h"
#include "roles.h"
#include <algorithm>
#include <functional>
#include <QFileInfo>
#include <QUrl>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QTimer>
#include <QtConcurrentRun>
#include <QDir>
#include <util/xpc/util.h>
#include <util/sys/paths.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <interfaces/idownload.h>
#include <interfaces/core/ientitymanager.h>
#include "icecastmodel.h"

namespace LC
{
namespace HotStreams
{
	const QString XiphFilename ("yp.xml");

	namespace
	{
		QString GetFilePath ()
		{
			return Util::GetUserDir (Util::UserDir::Cache, "hotstreams").filePath (XiphFilename);
		}

		bool ShouldUpdateFile (const QString& path)
		{
			return QFileInfo (path).lastModified ().daysTo (QDateTime::currentDateTime ()) > 2;
		}
	}

	IcecastFetcher::IcecastFetcher (IcecastModel *model, const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject (parent)
	, Model_ (model)
	{
		const auto& fullPath = GetFilePath ();
		const auto exists = QFile::exists (fullPath);
		if (!exists || ShouldUpdateFile (fullPath))
		{
			if (exists)
				QFile::remove (fullPath);
			FetchList (proxy);
		}
		else
			ParseList ();
	}

	void IcecastFetcher::FetchList (const ICoreProxy_ptr& proxy)
	{
		const auto& entity = Util::MakeEntity (QUrl ("http://dir.xiph.org/yp.xml"),
				GetFilePath (),
				OnlyDownload |
					Internal |
					DoNotAnnounceEntity |
					DoNotNotifyUser |
					DoNotSaveInHistory);
		const auto& res = proxy->GetEntityManager ()->DelegateEntity (entity);
		if (!res)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to delegate entity";
			deleteLater ();
			return;
		}

		Util::Sequence (this, res.DownloadResult_) >>
				Util::Visitor
				{
					[this] (IDownload::Success) { ParseList (); },
					[this] (const IDownload::Error&)
					{
						qWarning () << Q_FUNC_INFO
								<< "error fetching the list";
						deleteLater ();
					}
				};
	}

	namespace
	{
		void SortInfoList (QList<IcecastModel::StationInfo>& infos)
		{
			std::sort (infos.begin (), infos.end (), Util::ComparingBy (&IcecastModel::StationInfo::Name_));
		}

		void CoalesceOthers (QHash<QString, QList<IcecastModel::StationInfo>>& stations, int count)
		{
			auto lengths = Util::Map (stations, [] (const auto& list) { return list.size (); });
			std::sort (lengths.begin (), lengths.end (), std::greater<> ());
			const int threshold = lengths.at (count);

			QList<IcecastModel::StationInfo> otherInfos;
			for (const auto& genre : stations.keys ())
			{
				auto& genreStations = stations [genre];
				if (genreStations.size () <= threshold && genre != "metal")
				{
					otherInfos += genreStations;
					stations.remove (genre);
				}
				else
					SortInfoList (genreStations);
			}
			SortInfoList (otherInfos);
			stations ["Other"] = otherInfos;
		}

		IcecastModel::StationInfo ParseStationEntry (QXmlStreamReader& reader)
		{
			IcecastModel::StationInfo info;

			while (!reader.atEnd ())
			{
				switch (reader.readNext ())
				{
				case QXmlStreamReader::StartElement:
				{
					const auto& elementName = reader.name ();
					auto readField = [&elementName, &reader] (const QLatin1String& tagName, QString& field)
					{
						if (elementName != tagName)
							return false;

						field = reader.readElementText (QXmlStreamReader::ErrorOnUnexpectedElement);
						return true;
					};

					auto readAct = [&elementName, &reader] (const QLatin1String& tagName, const auto& action)
					{
						if (elementName != tagName)
							return false;

						action (reader.readElementText (QXmlStreamReader::ErrorOnUnexpectedElement));
						return true;
					};

					readField (QLatin1String { "server_name" }, info.Name_) ||
						readField (QLatin1String { "genre" }, info.Genre_) ||
						readAct (QLatin1String { "bitrate" }, [&info] (const QString& str) { info.Bitrate_ = str.toInt (); }) ||
						readAct (QLatin1String { "listen_url" }, [&info] (const QString& str) { info.URLs_ << QUrl { str }; }) ||
						readField (QLatin1String { "server_type" }, info.MIME_);
					break;
				}
				case QXmlStreamReader::EndElement:
					if (reader.qualifiedName () == "entry")
						return info;
				default:
					break;
				}
			}

			return info;
		}

		QHash<QString, QList<IcecastModel::StationInfo>> ParseStationsXml (QFile& file)
		{
			QHash<QString, QList<IcecastModel::StationInfo>> stations;

			QXmlStreamReader reader { &file };
			while (!reader.atEnd ())
			{
				switch (reader.readNext ())
				{
				case QXmlStreamReader::StartElement:
					if (reader.qualifiedName () == "entry")
					{
						const auto& info = ParseStationEntry (reader);
						const auto& genre = info.Genre_;
						auto& genreStations = stations [genre];
						const auto pos = std::find_if (genreStations.begin (), genreStations.end (),
								[&info] (const IcecastModel::StationInfo& other)
								{
									return info.Name_ == other.Name_ &&
											info.Bitrate_ == other.Bitrate_ &&
											info.MIME_ == other.MIME_;
								});
						if (pos == genreStations.end ())
							genreStations << info;
						else
							pos->URLs_ += info.URLs_;
					}
					break;
				default:
					continue;
				}
			}

			if (reader.hasError ())
			{
				qWarning () << Q_FUNC_INFO
						<< "parse failure:"
						<< reader.errorString ()
						<< ", removing the file";
				file.remove ();
				return {};
			}

			return stations;
		}

		IcecastModel::StationInfoList_t ParseWorker ()
		{
			QFile file { GetFilePath () };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open file";
				return {};
			}

			auto stations = ParseStationsXml (file);
			if (stations.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "empty stations";
				return {};
			}

			if (stations.size () > 20)
				CoalesceOthers (stations, 20);

			return Util::Map (Util::StlizeCopy<QPair> (stations), Util::Id);
		}
	}

	void IcecastFetcher::ParseList ()
	{
		Model_->SetStations ({});

		Util::Sequence (this, QtConcurrent::run (ParseWorker)) >>
				[this] (const IcecastModel::StationInfoList_t& list)
				{
					Model_->SetStations (list);
					deleteLater ();
				};
	}
}
}
