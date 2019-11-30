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

#include "description.h"
#include <stdexcept>
#include <type_traits>
#include <QDataStream>
#include <QUrlQuery>

namespace LC::SeekThru
{
	QDataStream& operator<< (QDataStream& out, const UrlDescription& d)
	{
		quint8 version = 1;
		out << version
			<< d.Template_
			<< d.Type_
			<< d.IndexOffset_
			<< d.PageOffset_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, UrlDescription& d)
	{
		quint8 version = 0;
		in >> version;
		if (version != 1)
			throw std::runtime_error ("Unknown version for UrlDescription");
		in >> d.Template_
			>> d.Type_
			>> d.IndexOffset_
			>> d.PageOffset_;
		return in;
	}

	QUrl UrlDescription::MakeUrl (const QString& searchStr, const QHash<QString, QVariant>& params) const
	{
		QUrl url = Template_;

		const auto& items = QUrlQuery { url }.queryItems ();

		std::decay<decltype (items)>::type newItems;
		for (auto item : items)
		{
			// Currently skips optional parameters
			if (item.second.size () >= 3 &&
					item.second.at (0) == '{' &&
					item.second.at (item.second.size () - 1) == '}' &&
					item.second.at (item.second.size () - 2) == '?')
				continue;

			if (item.second == "{searchTerms}")
				item.second = searchStr;
			else if (item.second.size () > 2 &&
					*item.second.begin () == '{' &&
					*(item.second.end () - 1) == '}')
			{
				auto key = item.second.mid (1, item.second.size () - 2);
				// To the correct string if Params_ has this key or to
				// empty string otherwise.
				item.second = params [key].toString ();
			}

			newItems << item;
		}
		QUrlQuery newQuery;
		newQuery.setQueryItems (newItems);
		url.setQuery (newQuery);
		return url;
	}

	QDataStream& operator<< (QDataStream& out, const QueryDescription& d)
	{
		quint8 version = 1;
		out << version
			<< static_cast<quint8> (d.Role_)
			<< d.Title_
			<< d.TotalResults_
			<< d.SearchTerms_
			<< d.Count_
			<< d.StartIndex_
			<< d.StartPage_
			<< d.Language_
			<< d.InputEncoding_
			<< d.OutputEncoding_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, QueryDescription& d)
	{
		quint8 version = 0;
		in >> version;
		if (version != 1)
			throw std::runtime_error ("Unknown version for QueryDescription");
		quint8 role;
		in >> role;
		d.Role_ = static_cast<QueryDescription::Role> (role);
		in >> d.Title_
			>> d.TotalResults_
			>> d.SearchTerms_
			>> d.Count_
			>> d.StartIndex_
			>> d.StartPage_
			>> d.Language_
			>> d.InputEncoding_
			>> d.OutputEncoding_;
		return in;
	}

	QDataStream& operator<< (QDataStream& out, const Description& d)
	{
		quint8 version = 1;
		out << version
			<< d.ShortName_
			<< d.Description_
			<< d.URLs_
			<< d.Contact_
			<< d.Tags_
			<< d.Queries_
			<< d.Developer_
			<< d.Attribution_
			<< static_cast<quint8> (d.Right_)
			<< d.Adult_
			<< d.Languages_
			<< d.InputEncodings_
			<< d.OutputEncodings_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, Description& d)
	{
		quint8 version = 0;
		in >> version;
		if (version != 1)
			throw std::runtime_error ("Unknown version for Description");
		in >> d.ShortName_
			>> d.Description_
			>> d.URLs_
			>> d.Contact_
			>> d.Tags_
			>> d.Queries_
			>> d.Developer_
			>> d.Attribution_;
		quint8 sr;
		in >> sr;
		d.Right_ = static_cast<Description::SyndicationRight> (sr);
		in >> d.Adult_
			>> d.Languages_
			>> d.InputEncodings_
			>> d.OutputEncodings_;
		return in;
	}
}
