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

#include "sqlstoragebackend.h"
#include <stdexcept>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QtDebug>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/db/oral/oral.h>
#include <util/db/oral/pgimpl.h>
#include <util/util.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Poshuku
{
	namespace oral = Util::oral;
	namespace sph = Util::oral::sph;

	struct SQLStorageBackend::History
	{
		oral::PKey<QDateTime, oral::NoAutogen> Date_;
		QString Title_;
		QString URL_;

		static QString ClassName ()
		{
			return "History";
		}

		using Indices = oral::Indices<
				oral::Index<&History::Title_, &History::URL_>
			>;

		HistoryItem ToHistoryItem () const
		{
			return { Title_, *Date_, URL_ };
		}

		static History FromHistoryItem (const HistoryItem& item)
		{
			return
			{
				item.DateTime_,
				item.Title_,
				item.URL_
			};
		}
	};

	struct SQLStorageBackend::Favorites
	{
		oral::PKey<QString, oral::NoAutogen> Title_;
		QString URL_;
		QString Tags_;

		static QString ClassName ()
		{
			return "Favorites";
		}

		FavoritesModel::FavoritesItem ToFavoritesItem () const
		{
			return
			{
				Title_,
				URL_,
				Tags_.split (" ", QString::SkipEmptyParts)
			};
		}

		static Favorites FromFavoritesItem (const FavoritesModel::FavoritesItem& item)
		{
			return
			{
				item.Title_,
				item.URL_,
				item.Tags_.join (" ")
			};
		}
	};

	struct SQLStorageBackend::FormsNever
	{
		oral::PKey<QString, oral::NoAutogen> URL_;

		static QString ClassName ()
		{
			return "Forms_Never";
		}
	};
}
}

BOOST_HANA_ADAPT_STRUCT (LeechCraft::Poshuku::SQLStorageBackend::History,
		Date_,
		Title_,
		URL_);
BOOST_HANA_ADAPT_STRUCT (LeechCraft::Poshuku::SQLStorageBackend::Favorites,
		Title_,
		URL_,
		Tags_);
BOOST_HANA_ADAPT_STRUCT (LeechCraft::Poshuku::SQLStorageBackend::FormsNever,
		URL_);

namespace LeechCraft
{
namespace Poshuku
{
	SQLStorageBackend::SQLStorageBackend (StorageBackend::Type type)
	: DBGuard_ { Util::MakeScopeGuard ([this] { DB_.close (); }) }
	{
		QString strType;
		switch (type)
		{
		case SBSQLite:
			strType = "QSQLITE";
			break;
		case SBPostgres:
			strType = "QPSQL";
			break;
		}

		DB_ = QSqlDatabase::addDatabase (strType, Util::GenConnectionName ("org.LeechCraft.Poshuku"));
		switch (type)
		{
		case SBSQLite:
		{
			QDir dir = QDir::home ();
			dir.cd (".leechcraft");
			dir.cd ("poshuku");
			DB_.setDatabaseName (dir.filePath ("poshuku.db"));
			break;
		}
		case SBPostgres:
		{
			DB_.setDatabaseName (XmlSettingsManager::Instance ()->property ("PostgresDBName").toString ());
			DB_.setHostName (XmlSettingsManager::Instance ()->property ("PostgresHostname").toString ());
			DB_.setPort (XmlSettingsManager::Instance ()->property ("PostgresPort").toInt ());
			DB_.setUserName (XmlSettingsManager::Instance ()->property ("PostgresUsername").toString ());
			DB_.setPassword (XmlSettingsManager::Instance ()->property ("PostgresPassword").toString ());
			break;
		}
		}

		if (!DB_.open ())
		{
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error (QString ("Could not initialize database: %1")
					.arg (DB_.lastError ().text ()).toUtf8 ().constData ());
		}

		if (type == SBSQLite)
			Util::RunTextQuery (DB_, "PRAGMA journal_model = WAL;");

		auto adaptedPtrs = std::tie (History_, Favorites_, FormsNever_);
		type == SBSQLite ?
				oral::AdaptPtrs<oral::SQLiteImplFactory> (DB_, adaptedPtrs) :
				oral::AdaptPtrs<oral::PostgreSQLImplFactory> (DB_, adaptedPtrs);
	}

	void SQLStorageBackend::LoadHistory (history_items_t& items) const
	{
		for (const auto& item : History_->Select.Build ().Order (oral::OrderBy<sph::desc<&History::Date_>>) ())
			items.push_back (item.ToHistoryItem ());
	}

	void SQLStorageBackend::LoadResemblingHistory (const QString& base,
			history_items_t& items) const
	{
		using namespace oral::infix;

		const auto& pat = "%" + base + "%";
		const auto& allItems = History_->Select.Build ()
				.Select (sph::fields<&History::Title_, &History::Date_>)
				.Where (sph::f<&History::Title_> |like| pat || sph::f<&History::URL_> |like| pat)
				.Order (oral::OrderBy<sph::desc<&History::Date_>>)
				();
	}

	void SQLStorageBackend::AddToHistory (const HistoryItem& item)
	{
		History_->Insert (History::FromHistoryItem (item), oral::InsertAction::Replace::PKey<History>);
		emit added (item);
	}

	void SQLStorageBackend::ClearOldHistory (int age, int items)
	{
		const auto& countDateThreshold = History_->SelectOne
				.Build ()
				.Select (sph::fields<&History::Date_>)
				.Order (oral::OrderBy<sph::asc<&History::Date_>>)
				.Limit (1)
				.Offset (items)
				();

		const auto& ageDateThreshold = QDateTime::currentDateTime ().addDays (-age);

		const auto& threshold = countDateThreshold ?
				std::min (*countDateThreshold, ageDateThreshold) :
				ageDateThreshold;
		History_->DeleteBy (sph::f<&History::Date_> < threshold);
	}

	void SQLStorageBackend::LoadFavorites (FavoritesModel::items_t& items) const
	{
		for (const auto& fav : Favorites_->Select.Build ().Order (oral::OrderBy<sph::desc<&Favorites::Title_>>) ())
			items.push_back (fav.ToFavoritesItem ());
	}

	void SQLStorageBackend::AddToFavorites (const FavoritesModel::FavoritesItem& item)
	{
		Favorites_->Insert (Favorites::FromFavoritesItem (item));
		emit added (item);
	}

	void SQLStorageBackend::RemoveFromFavorites (const FavoritesModel::FavoritesItem& item)
	{
		Favorites_->DeleteBy (sph::f<&Favorites::URL_> == item.URL_);
		emit removed (item);
	}

	void SQLStorageBackend::UpdateFavorites (const FavoritesModel::FavoritesItem& item)
	{
		Favorites_->Update (Favorites::FromFavoritesItem (item));
		emit updated (item);
	}

	void SQLStorageBackend::SetFormsIgnored (const QString& url, bool ignore)
	{
		if (ignore)
			FormsNever_->Insert ({ url });
		else
			FormsNever_->Delete ({ url });
	}

	bool SQLStorageBackend::GetFormsIgnored (const QString& url) const
	{
		return FormsNever_->Select (sph::count<>, sph::f<&FormsNever::URL_> == url);
	}
}
}
