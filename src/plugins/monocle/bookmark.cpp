/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmark.h"
#include <QDataStream>
#include <QDomElement>
#include <QtDebug>

namespace LC
{
namespace Monocle
{
	Bookmark::Bookmark (const QString& name, int page, const QPoint& position)
	: Name_ (name)
	, Page_ (page)
	, Position_ (position)
	{
	}

	QString Bookmark::GetName () const
	{
		return Name_;
	}

	void Bookmark::SetName (const QString& name)
	{
		Name_ = name;
	}

	int Bookmark::GetPage () const
	{
		return Page_;
	}

	void Bookmark::SetPage (int page)
	{
		Page_ = page;
	}

	QPoint Bookmark::GetPosition () const
	{
		return Position_;
	}

	void Bookmark::SetPosition (const QPoint& p)
	{
		Position_ = p;
	}

	void Bookmark::ToXML (QDomElement& elem, QDomDocument& doc) const
	{
		auto pageElem = doc.createElement ("page");
		pageElem.setAttribute ("num", Page_);
		elem.appendChild (pageElem);

		auto posElem = doc.createElement ("pos");
		posElem.setAttribute ("x", Position_.x ());
		posElem.setAttribute ("y", Position_.y ());
		elem.appendChild (posElem);

		elem.setAttribute ("name", Name_);
	}

	Bookmark Bookmark::FromXML (const QDomElement& elem)
	{
		const auto page = elem.firstChildElement ("page").attribute ("num").toInt ();
		const auto& posElem = elem.firstChildElement ("pos");
		const auto& name = elem.attribute ("name");
		return
		{
			name,
			page,
			{ posElem.attribute ("x").toInt (), posElem.attribute ("y").toInt () }
		};
	}

	bool operator== (const Bookmark& b1, const Bookmark& b2)
	{
		return b1.GetPosition () == b2.GetPosition () &&
			b1.GetName () == b2.GetName ();
	}
}
}
