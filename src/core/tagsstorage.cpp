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

#include "tagsstorage.h"
#include <QDir>
#include <QSqlError>
#include <util/db/oral/oral.h>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/sll/prelude.h>
#include <util/sys/paths.h>

namespace LC
{
	struct TagsStorage::Record
	{
		Util::oral::Unique<Util::oral::NotNull<QByteArray>> Id_;
		Util::oral::NotNull<QString> Name_;

		static QString ClassName ()
		{
			return "Tags";
		}
	};
}

BOOST_FUSION_ADAPT_STRUCT (LC::TagsStorage::Record,
		Id_,
		Name_)

namespace LC
{
	namespace sph = Util::oral::sph;

	TagsStorage::TagsStorage (QObject *parent)
	: QObject { parent }
	, DB_ { QSqlDatabase::addDatabase ("QSQLITE",
			Util::GenConnectionName ("org.LeechCraft.Core.TagsStorage")) }
	{
		const auto& coreDir = Util::GetUserDir (Util::UserDir::LC, "core");
		DB_.setDatabaseName (coreDir.filePath ("core.db"));

		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		Record_ = Util::oral::AdaptPtr<Record> (DB_);
	}

	void TagsStorage::AddTag (const Id& id, const QString& name)
	{
		Record_->Insert ({ id.toByteArray (), name });
	}

	void TagsStorage::DeleteTag (const Id& id)
	{
		Record_->DeleteBy (sph::f<&Record::Id_> == id.toByteArray ());
	}

	void TagsStorage::SetTagName (const Id& id, const QString& newName)
	{
		Record_->Update (sph::f<&Record::Name_> = newName,
				sph::f<&Record::Id_> == id.toByteArray ());
	}

	QList<QPair<TagsStorage::Id, QString>> TagsStorage::GetAllTags () const
	{
		return Util::Map (Record_->Select (),
				[] (const Record& rec) { return QPair { QUuid::fromString (QString::fromLatin1 (**rec.Id_)), *rec.Name_ }; });
	}
}