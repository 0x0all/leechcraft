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

#pragma once

#include <util/sll/visitor.h>
#include "oraltypes.h"
#include "oraldetailfwd.h"
#include "impldefs.h"

namespace LeechCraft::Util::oral::detail::SQLite
{
	using QSqlQuery_ptr = std::shared_ptr<QSqlQuery>;

	class InsertQueryBuilder final : public IInsertQueryBuilder
	{
		const QSqlDatabase DB_;

		std::array<QSqlQuery_ptr, InsertAction::StaticCount ()> Queries_;
		const QString InsertSuffix_;
	public:
		InsertQueryBuilder (const QSqlDatabase& db, const CachedFieldsData& data)
		: DB_ { db }
		, InsertSuffix_ { " INTO " + data.Table_ +
			" (" + data.Fields_.join (", ") + ") VALUES (" +
			data.BoundFields_.join (", ") + ");" }
		{
		}

		QSqlQuery_ptr GetQuery (InsertAction action) override
		{
			auto& query = Queries_ [std::min (action.Selector_.which (), InsertAction::StaticCount () - 1)];
			if (!query)
			{
				query = std::make_shared<QSqlQuery> (DB_);
				query->prepare (GetInsertPrefix (action) + InsertSuffix_);
			}
			return query;
		}
	private:
		QString GetInsertPrefix (InsertAction action)
		{
			return Visit (action.Selector_,
					[] (InsertAction::DefaultTag) { return "INSERT"; },
					[] (InsertAction::IgnoreTag) { return "INSERT OR IGNORE"; },
					[] (InsertAction::ReplaceTag) { return "INSERT OR REPLACE"; },
					[] (InsertAction::ReplaceByConstraint) { return "INSERT OR REPLACE"; });
		}
	};

	class ImplFactory
	{
	public:
		struct TypeLits
		{
			inline static const QString IntAutoincrement { "INTEGER PRIMARY KEY AUTOINCREMENT" };
		};

		auto MakeInsertQueryBuilder (const QSqlDatabase& db, const CachedFieldsData& data) const
		{
			return std::make_unique<InsertQueryBuilder> (db, data);
		}
	};
}

namespace LeechCraft::Util::oral
{
	using SQLiteImplFactory = detail::SQLite::ImplFactory;
}
