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

#include "logger.h"
#include <QDir>
#include <QSqlError>
#include <util/db/oral/oral.h>
#include <util/db/util.h>
#include <util/sll/unreachable.h>
#include <util/sys/paths.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iaccount.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Herbicide
{
	struct Logger::AccountRecord
	{
		Util::oral::PKey<int> PKey_;

		Util::oral::Unique<QString> AccountID_;
		QString AccountName_;

		static QByteArray ClassName ()
		{
			return "AccountRecord";
		}
	};

	struct Logger::EntryRecord
	{
		Util::oral::PKey<int> PKey_;
		Util::oral::References<&AccountRecord::PKey_> AccountID_;

		Util::oral::Unique<QString> EntryID_;
		QString EntryHumanReadableId_;
		QString EntryName_;

		static QByteArray ClassName ()
		{
			return "EntryRecord";
		}
	};

	struct Logger::EventRecord
	{
		Util::oral::PKey<int> PKey_;
		Util::oral::References<&EntryRecord::PKey_> EntryID_;

		Logger::Event Event_;
		QString Reason_;

		static QByteArray ClassName ()
		{
			return "EventRecord";
		}
	};
}
}
}

BOOST_HANA_ADAPT_STRUCT (LeechCraft::Azoth::Herbicide::Logger::AccountRecord,
		PKey_,
		AccountID_,
		AccountName_);

BOOST_HANA_ADAPT_STRUCT (LeechCraft::Azoth::Herbicide::Logger::EntryRecord,
		PKey_,
		AccountID_,
		EntryID_,
		EntryHumanReadableId_,
		EntryName_);

BOOST_HANA_ADAPT_STRUCT (LeechCraft::Azoth::Herbicide::Logger::EventRecord,
		PKey_,
		EntryID_,
		Event_,
		Reason_);

namespace LeechCraft
{
namespace Util
{
namespace oral
{
	template<typename ImplFactory>
	struct Type2Name<ImplFactory, Azoth::Herbicide::Logger::Event>
	{
		QString operator() () const
		{
			return Type2Name<ImplFactory, QString> {} ();
		}
	};

	template<>
	struct ToVariant<Azoth::Herbicide::Logger::Event>
	{
		QVariant operator() (Azoth::Herbicide::Logger::Event event) const
		{
			switch (event)
			{
			case Azoth::Herbicide::Logger::Event::Granted:
				return "granted";
			case Azoth::Herbicide::Logger::Event::Denied:
				return "denied";
			case Azoth::Herbicide::Logger::Event::Challenged:
				return "challenged";
			case Azoth::Herbicide::Logger::Event::Succeeded:
				return "succeeded";
			case Azoth::Herbicide::Logger::Event::Failed:
				return "failed";
			}

			Util::Unreachable ();
		}
	};

	template<>
	struct FromVariant<Azoth::Herbicide::Logger::Event>
	{
		Azoth::Herbicide::Logger::Event operator() (const QVariant& var) const
		{
			static const QMap<QString, Azoth::Herbicide::Logger::Event> map
			{
				{ "granted", Azoth::Herbicide::Logger::Event::Granted },
				{ "denied", Azoth::Herbicide::Logger::Event::Denied },
				{ "challenged", Azoth::Herbicide::Logger::Event::Challenged },
				{ "succeeded", Azoth::Herbicide::Logger::Event::Succeeded },
				{ "failed", Azoth::Herbicide::Logger::Event::Failed },
			};
			return map.value (var.toString ());
		}
	};
}
}

namespace Azoth
{
namespace Herbicide
{
	Logger::Logger (QObject *parent)
	: QObject { parent }
	, DB_ { QSqlDatabase::addDatabase ("QSQLITE",
				Util::GenConnectionName ("org.LeechCraft.Azoth.Herbicide.Log")) }
	{
		const auto& herbicideDir = Util::GetUserDir (Util::UserDir::LC, "azoth/herbicide");
		DB_.setDatabaseName (herbicideDir.filePath ("log.db"));
		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		AdaptedAccount_ = Util::oral::AdaptPtr<AccountRecord> (DB_);
		AdaptedEntry_ = Util::oral::AdaptPtr<EntryRecord> (DB_);
		AdaptedEvent_ = Util::oral::AdaptPtr<EventRecord> (DB_);
	}

	namespace sph = Util::oral::sph;

	void Logger::LogEvent (Logger::Event event, const ICLEntry *entry, const QString& descr)
	{
		const QString accId { entry->GetParentAccount ()->GetAccountID () };
		const auto& entryId = entry->GetEntryID ();

		const auto& maybeAccPKey = AdaptedAccount_->SelectOne (sph::_0, sph::_1 == accId);
		const auto accPKey = maybeAccPKey ?
				*maybeAccPKey :
				InsertAccount (entry->GetParentAccount ());

		const auto maybeEntryPKey = AdaptedEntry_->SelectOne (sph::_0, sph::_2 == entryId);
		const auto entryPKey = maybeEntryPKey ?
				*maybeEntryPKey :
				InsertEntry (accPKey, entry);

		AdaptedEvent_->Insert ({ {}, entryPKey, event, descr });
	}

	int Logger::InsertAccount (const IAccount *acc)
	{
		return AdaptedAccount_->Insert ({
				{},
				QString { acc->GetAccountID () },
				acc->GetAccountName ()
			});
	}

	int Logger::InsertEntry (int accPKey, const ICLEntry *entry)
	{
		return AdaptedEntry_->Insert ({
				{},
				accPKey,
				entry->GetEntryID (),
				entry->GetHumanReadableID (),
				entry->GetEntryName ()
			});
	}
}
}
}
