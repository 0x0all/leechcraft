/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "todoitem.h"
#include <QUuid>
#include <QDataStream>
#include <QtDebug>
#include <interfaces/core/itagsmanager.h>
#include "core.h"

namespace LC
{
namespace Otlozhu
{
	TodoItem::TodoItem ()
	: ID_ (QUuid::createUuid ().toString ())
	{
	}

	TodoItem::TodoItem (const QString& id)
	: ID_ (id)
	{
	}

	TodoItem_ptr TodoItem::Clone () const
	{
		TodoItem_ptr clone (new TodoItem (GetID ()));
		clone->Title_ = Title_;
		clone->Comment_ = Comment_;
		clone->TagIDs_ = TagIDs_;
		clone->Created_ = Created_;
		clone->Due_ = Due_;
		clone->Percentage_ = Percentage_;
		clone->Deps_ = Deps_;
		return clone;
	}

	void TodoItem::CopyFrom (const TodoItem_ptr item)
	{
		Title_ = item->Title_;
		Comment_ = item->Comment_;
		TagIDs_ = item->TagIDs_;
		Created_ = item->Created_;
		Due_ = item->Due_;
		Percentage_ = item->Percentage_;
		Deps_ = item->Deps_;
	}

	QVariantMap TodoItem::ToMap () const
	{
		const auto& tags = Core::Instance ().GetProxy ()->GetTagsManager ()->GetTags (TagIDs_);
		return
		{
				{ "Title", Title_ },
				{ "Comment", Comment_ },
				{ "Tags", tags },
				{ "Deps", Deps_ },
				{ "Created", Created_ },
				{ "Due", Due_ },
				{ "Percentage", Percentage_ }
		};
	}

	QVariantMap TodoItem::DiffWith (const TodoItem_ptr item) const
	{
		const auto& thatMap = item->ToMap ();
		auto thisMap = ToMap ();
		for (auto i = thisMap.begin (); i != thisMap.end ();)
		{
			const auto& key = i.key ();
			if (thisMap [key] == thatMap [key])
				i = thisMap.erase (i);
			else
				++i;
		}
		return thisMap;
	}

	namespace
	{
		class Applier
		{
			TodoItem * const Item_;

			const QVariantMap& Map_;
		public:
			Applier (TodoItem *item, const QVariantMap& map)
			: Item_ (item)
			, Map_ (map)
			{
			}

			template<typename T>
			Applier& operator() (const QString& name, T TodoItem::* g)
			{
				if (Map_.contains (name))
					Item_->*g = Map_ [name].value<T> ();
				return *this;
			}

			template<typename T>
			Applier& operator() (const QString& name, const T& g)
			{
				if (Map_.contains (name))
					g (Item_, Map_ [name]);
				return *this;
			}
		};
	}

	void TodoItem::ApplyDiff (const QVariantMap& map)
	{
		Applier { this, map }
				("Title", &TodoItem::Title_)
				("Comment", &TodoItem::Comment_)
				("Tags",
					[] (TodoItem *item, const QVariant& tagsVar)
					{
						item->TagIDs_ = Core::Instance ().GetProxy ()->
								GetTagsManager ()->GetIDs (tagsVar.toStringList ());
					})
				("Deps", &TodoItem::Deps_)
				("Created", [] (TodoItem *item, QVariant time)
						{ item->Created_.setSecsSinceEpoch (time.toUInt ()); })
				("Due", [] (TodoItem *item, QVariant time)
						{ item->Due_.setSecsSinceEpoch (time.toUInt ()); })
				("Percentage", &TodoItem::Percentage_);
	}

	TodoItem_ptr TodoItem::Deserialize (const QByteArray& data)
	{
		QDataStream str (data);
		quint8 version = 0;
		str >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return TodoItem_ptr ();
		}

		TodoItem_ptr item (new TodoItem);
		str >> item->ID_
			>> item->Title_
			>> item->Comment_
			>> item->TagIDs_
			>> item->Created_
			>> item->Due_
			>> item->Percentage_
			>> item->Deps_;
		return item;
	}

	QByteArray TodoItem::Serialize () const
	{
		QByteArray res;
		QDataStream out (&res, QIODevice::WriteOnly);

		out << static_cast<quint8> (1)
			<< ID_
			<< Title_
			<< Comment_
			<< TagIDs_
			<< Created_
			<< Due_
			<< Percentage_
			<< Deps_;

		return res;
	}

	QString TodoItem::GetID () const
	{
		return ID_;
	}

	QString TodoItem::GetTitle () const
	{
		return Title_;
	}

	void TodoItem::SetTitle (const QString& title)
	{
		Title_ = title;
	}

	QString TodoItem::GetComment () const
	{
		return Comment_;
	}

	void TodoItem::SetComment (const QString& comment)
	{
		Comment_ = comment;
	}

	QStringList TodoItem::GetTagIDs () const
	{
		return TagIDs_;
	}

	void TodoItem::SetTagIDs (const QStringList& tagIds)
	{
		TagIDs_ = tagIds;
	}

	QDateTime TodoItem::GetCreatedDate () const
	{
		return Created_;
	}

	void TodoItem::SetCreatedDate (const QDateTime& created)
	{
		Created_ = created;
	}

	QDateTime TodoItem::GetDueDate () const
	{
		return Due_;
	}

	void TodoItem::SetDueDate (const QDateTime& due)
	{
		Due_ = due;
	}

	int TodoItem::GetPercentage () const
	{
		return Percentage_;
	}

	void TodoItem::SetPercentage (int p)
	{
		Percentage_ = p;
	}

	QStringList TodoItem::GetDeps () const
	{
		return Deps_;
	}

	void TodoItem::SetDeps (const QStringList& deps)
	{
		Deps_ = deps;
	}

	void TodoItem::AddDep (const QString& dep)
	{
		Deps_ << dep;
	}
}
}
