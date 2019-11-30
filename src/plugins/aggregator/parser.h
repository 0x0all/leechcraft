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

#ifndef PLUGINS_AGGREGATOR_PARSER_H
#define PLUGINS_AGGREGATOR_PARSER_H
#include <vector>
#include <QPair>
#include <QDomDocument>
#include "channel.h"

namespace LC
{
namespace Aggregator
{
	class Parser
	{
		friend class MRSSParser;
	public:
		virtual ~Parser () = default;
		/** @brief Indicates whether parser could parse the document.
			*
			* @param[in] doc The document to parse.
			* @return Whether the given document \em doc can be parsed.
			*/
		virtual bool CouldParse (const QDomDocument& doc) const = 0;

		/** @brief Parses the document
			*
			* Parses the passed XML document. Created channels are
			* already sane and validated, with proper feed IDs and
			* such (that's why feedId parameter is required).
			*
			* @param[in] document Byte array with XML document.
			* @param[in] feedId The ID of the parent feed.
			* @return Container (channels_container_t) with new items.
			*/
		virtual channels_container_t ParseFeed (const QDomDocument& document,
				const IDType_t& feedId) const;
	protected:
		static const QString DC_;
		static const QString WFW_;
		static const QString Atom_;
		static const QString RDF_;
		static const QString Slash_;
		static const QString Enc_;
		static const QString ITunes_;
		static const QString GeoRSSSimple_;
		static const QString GeoRSSW3_;
		static const QString MediaRSS_;
		static const QString Content_;

		virtual channels_container_t Parse (const QDomDocument&,
				const IDType_t&) const = 0;
		QString GetDescription (const QDomElement&) const;
		void GetDescription (const QDomElement&, QString&) const;
		QString GetLink (const QDomElement&) const;
		QString GetAuthor (const QDomElement&) const;
		QString GetCommentsRSS (const QDomElement&) const;
		QString GetCommentsLink (const QDomElement&) const;
		int GetNumComments (const QDomElement&) const;
		QDateTime GetDCDateTime (const QDomElement&) const;
		QStringList GetAllCategories (const QDomElement&) const;
		QStringList GetDCCategories (const QDomElement&) const;
		QStringList GetITunesCategories (const QDomElement&) const;
		QStringList GetPlainCategories (const QDomElement&) const;
		QList<Enclosure> GetEncEnclosures (const QDomElement&,
				const IDType_t&) const;
		QPair<double, double> GetGeoPoint (const QDomElement&) const;
		QList<MRSSEntry> GetMediaRSS (const QDomElement&,
				const IDType_t&) const;

		QDateTime FromRFC3339 (const QString&) const;
		static QString UnescapeHTML (const QString&);
	};
}
}

#endif
