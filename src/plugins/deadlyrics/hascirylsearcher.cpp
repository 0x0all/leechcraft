/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hascirylsearcher.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QHttpPart>
#include <QHttpMultiPart>
#include <util/network/lcserviceoverride.h>
#include <util/network/handlenetworkreply.h>
#include <util/sll/functor.h>
#include <util/sll/parsejson.h>

namespace LC
{
namespace DeadLyrics
{
	namespace
	{
		namespace
		{
			QString GetUrl (const QString& path)
			{
				return Util::GetServiceUrl ({ "127.0.0.1", 12001, "DL_HASCIRYL", false }, path);
			}
		}
	}

	HascirylSearcher::HascirylSearcher (QNetworkAccessManager *nam)
	: NAM_ { nam }
	{
	}

	void HascirylSearcher::Search (const Media::LyricsQuery& query, const Reporter_t& reporter)
	{
		using Util::operator*;

		const QNetworkRequest req { GetUrl ("lyrics/pages/urls") };

		QByteArray postData;
		auto append = [&postData] (const QByteArray& key, const QString& value)
		{
			if (value.isEmpty ())
				return;

			if (!postData.isEmpty ())
				postData += '&';
			postData += key + '=' + QUrl::toPercentEncoding (value);
		};
		append ("artist", query.Artist_);
		append ("album", query.Album_);
		append ("title", query.Title_);
		auto num = [] (int num) { return QString::number (num); };
		append ("year", (num * query.Year_).value_or (QString {}));
		append ("track", (num * query.Track_).value_or (QString {}));

		Util::HandleNetworkReply (this, NAM_->post (req, postData),
				[this, reporter] (const QByteArray& data) { HandleLyricsUrls (reporter, data); });
	}

	void HascirylSearcher::HandleLyricsUrls (const Reporter_t& reporter, const QByteArray& data)
	{
		const auto& urls = Util::ParseJson (data, Q_FUNC_INFO).toList ();
		for (const auto& varMap : urls)
		{
			const auto& map = varMap.toMap ();
			const auto& prov = map ["provName"].toString ();
			const auto& url = map ["reqUrl"].toString ();

			Util::HandleNetworkReply (this, NAM_->get (QNetworkRequest { QUrl { url } }),
					[this, reporter, prov] (const QByteArray& data) { HandleLyricsPageFetched (reporter, prov, data); });
		}
	}

	void HascirylSearcher::HandleLyricsPageFetched (const Reporter_t& reporter,
			const QString& provName, const QByteArray& data)
	{
		QHttpPart servicePart;
		servicePart.setHeader (QNetworkRequest::ContentDispositionHeader, "form-data; name=\"service\"");
		servicePart.setBody (provName.toUtf8 ());

		QHttpPart contentsPart;
		contentsPart.setHeader (QNetworkRequest::ContentDispositionHeader,
				"form-data; name=\"contents\"; filename=\"contents\"");
		contentsPart.setBody (data);

		auto multipart = new QHttpMultiPart { QHttpMultiPart::FormDataType };
		multipart->append (servicePart);
		multipart->append (contentsPart);

		const auto reply = NAM_->post (QNetworkRequest { GetUrl ("lyrics/page/parse") }, multipart);
		multipart->setParent (reply);

		Util::HandleNetworkReply (this, reply,
				[this, reporter, provName] (const QByteArray& data) { HandleGotLyricsReply (reporter, provName, data); });
	}

	void HascirylSearcher::HandleGotLyricsReply (const Reporter_t& reporter,
			const QString& provName, const QByteArray& data)
	{
		const auto& reply = Util::ParseJson (data, Q_FUNC_INFO).toMap ();
		const auto& result = reply ["result"].toString ();
		if (result != "Success")
		{
			reporter (Media::ILyricsFinder::LyricsQueryResult_t { tr ("Lyrics not found.") });
			return;
		}

		const auto& lyrics = reply ["payload"].toString ();
		reporter (Media::ILyricsFinder::LyricsQueryResult_t { { { provName, lyrics } } });
	}
}
}
