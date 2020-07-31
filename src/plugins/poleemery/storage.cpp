/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "storage.h"
#include <stdexcept>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QtDebug>
#include <util/db/dblock.h>
#include <util/db/oral/oral.h>
#include <util/sys/paths.h>
#include <util/sll/unreachable.h>

namespace LC
{
namespace Util
{
namespace oral
{
	template<>
	struct Type2Name<Poleemery::AccType>
	{
		QString operator() () const { return "TEXT"; }
	};

	template<>
	struct ToVariant<Poleemery::AccType>
	{
		QVariant operator() (const Poleemery::AccType& type) const
		{
			switch (type)
			{
			case Poleemery::AccType::BankAccount:
				return "BankAccount";
			case Poleemery::AccType::Cash:
				return "Cash";
			}

			Util::Unreachable ();
		}
	};

	template<>
	struct FromVariant<Poleemery::AccType>
	{
		Poleemery::AccType operator() (const QVariant& var) const
		{
			const auto& str = var.toString ();
			if (str == "BankAccount")
				return Poleemery::AccType::BankAccount;
			else if (str == "Cash")
				return Poleemery::AccType::Cash;
			else
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown account type"
						<< str;
				return Poleemery::AccType::Cash;
			}
		}
	};
}
}

namespace Poleemery
{
	struct StorageImpl
	{
		QSqlDatabase DB_;

		Util::oral::ObjectInfo_ptr<Account> AccountInfo_;
		Util::oral::ObjectInfo_ptr<NakedExpenseEntry> NakedExpenseEntryInfo_;
		Util::oral::ObjectInfo_ptr<ReceiptEntry> ReceiptEntryInfo_;
		Util::oral::ObjectInfo_ptr<Category> CategoryInfo_;
		Util::oral::ObjectInfo_ptr<CategoryLink> CategoryLinkInfo_;
		Util::oral::ObjectInfo_ptr<Rate> RateInfo_;

		QHash<QString, Category> CatCache_;
		QHash<int, Category> CatIDCache_;
	};

	Storage::Storage (QObject *parent)
	: QObject (parent)
	, Impl_ (new StorageImpl)
	{
		Impl_->DB_ = QSqlDatabase::addDatabase ("QSQLITE", "Poleemery_Connection");
		const auto& dir = Util::CreateIfNotExists ("poleemeery");
		Impl_->DB_.setDatabaseName (dir.absoluteFilePath ("database.db"));

		if (!Impl_->DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open database:"
					<< Impl_->DB_.lastError ().text ();
			throw std::runtime_error ("Poleemery database creation failed");
		}

		Util::RunTextQuery (Impl_->DB_, "PRAGMA foreign_keys = ON;");

		InitializeTables ();
		LoadCategories ();
	}

	QList<Account> Storage::GetAccounts () const
	{
		return Impl_->AccountInfo_->DoSelectAll_ ();
	}

	void Storage::AddAccount (Account& acc)
	{
		Impl_->AccountInfo_->DoInsert_ (acc);
	}

	void Storage::UpdateAccount (const Account& acc)
	{
		Impl_->AccountInfo_->DoUpdate_ (acc);
	}

	void Storage::DeleteAccount (const Account& acc)
	{
		Impl_->AccountInfo_->DoDelete_ (acc);
	}

	QList<ExpenseEntry> Storage::GetExpenseEntries ()
	{
		return HandleNaked (Impl_->NakedExpenseEntryInfo_->DoSelectAll_ ());
	}

	QList<ExpenseEntry> Storage::GetExpenseEntries (const Account& parent)
	{
		return HandleNaked (Impl_->NakedExpenseEntryInfo_->SelectByFKeysActor_ (boost::fusion::make_vector (parent.ID_)));
	}

	void Storage::AddExpenseEntry (ExpenseEntry& entry)
	{
		Util::DBLock lock (Impl_->DB_);
		lock.Init ();
		Impl_->NakedExpenseEntryInfo_->DoInsert_ (entry);
		AddNewCategories (entry, entry.Categories_);
		lock.Good ();
	}

	void Storage::UpdateExpenseEntry (const ExpenseEntry& entry)
	{
		Util::DBLock lock (Impl_->DB_);
		lock.Init ();

		Impl_->NakedExpenseEntryInfo_->DoUpdate_ (entry);

		auto nowCats = entry.Categories_;

		for (const auto& cat : boost::fusion::at_c<1> (Impl_->CategoryLinkInfo_->SingleFKeySelectors_) (entry.ID_))
			if (!nowCats.removeAll (Impl_->CatIDCache_.value (cat.Category_).Name_))
				Impl_->CategoryLinkInfo_->DoDelete_ (cat);

		if (!nowCats.isEmpty ())
			AddNewCategories (entry, nowCats);

		lock.Good ();
	}

	void Storage::DeleteExpenseEntry (const ExpenseEntry& entry)
	{
		Impl_->NakedExpenseEntryInfo_->DoDelete_ (entry);
	}

	QList<ReceiptEntry> Storage::GetReceiptEntries ()
	{
		return Impl_->ReceiptEntryInfo_->DoSelectAll_ ();
	}

	QList<ReceiptEntry> Storage::GetReceiptEntries (const Account& account)
	{
		return Impl_->ReceiptEntryInfo_->SelectByFKeysActor_ (boost::fusion::make_vector (account.ID_));
	}

	void Storage::AddReceiptEntry (ReceiptEntry& entry)
	{
		Impl_->ReceiptEntryInfo_->DoInsert_ (entry);
	}

	void Storage::UpdateReceiptEntry (const ReceiptEntry& entry)
	{
		Impl_->ReceiptEntryInfo_->DoUpdate_ (entry);
	}

	void Storage::DeleteReceiptEntry (const ReceiptEntry& entry)
	{
		Impl_->ReceiptEntryInfo_->DoDelete_ (entry);
	}

	QList<Rate> Storage::GetRates ()
	{
		return Impl_->RateInfo_->DoSelectAll_ ();
	}

	namespace sph = Util::oral::sph;

	QList<Rate> Storage::GetRates (const QDateTime& start, const QDateTime& end)
	{
		return Impl_->RateInfo_->DoSelectByFields_ (start < sph::_2 && sph::_2 < end);
	}

	QList<Rate> Storage::GetRate (const QString& code)
	{
		return Impl_->RateInfo_->DoSelectByFields_ (sph::_1 == code);
	}

	QList<Rate> Storage::GetRate (const QString& code, const QDateTime& start, const QDateTime& end)
	{
		return Impl_->RateInfo_->DoSelectByFields_ (sph::_1 == code && start < sph::_2 && sph::_2 < end);
	}

	void Storage::AddRate (Rate& rate)
	{
		Impl_->RateInfo_->DoInsert_ (rate);
	}

	Category Storage::AddCategory (const QString& name)
	{
		Category cat { name };
		Impl_->CategoryInfo_->DoInsert_ (cat);
		Impl_->CatCache_ [name] = cat;
		Impl_->CatIDCache_ [cat.ID_] = cat;
		return cat;
	}

	void Storage::AddNewCategories (const ExpenseEntry& entry, const QStringList& cats)
	{
		for (const auto& cat : cats)
		{
			if (!Impl_->CatCache_.contains (cat))
				AddCategory (cat);
			LinkEntry2Cat (entry, Impl_->CatCache_ [cat]);
		}
	}

	void Storage::LinkEntry2Cat (const ExpenseEntry& entry, const Category& category)
	{
		CategoryLink link (category, entry);
		Impl_->CategoryLinkInfo_->DoInsert_ (link);
	}

	void Storage::UnlinkEntry2Cat (const ExpenseEntry& entry, const Category& category)
	{
		const auto& link = Impl_->CategoryLinkInfo_->SelectByFKeysActor_ (boost::fusion::make_vector (category.ID_, entry.ID_));
		if (!link.isEmpty ())
			Impl_->CategoryLinkInfo_->DoDelete_ (link.first ());
	}

	QList<ExpenseEntry> Storage::HandleNaked (const QList<NakedExpenseEntry>& nakedItems)
	{
		QList<ExpenseEntry> entries;

		for (const auto& naked : nakedItems)
		{
			ExpenseEntry entry { naked };

			const auto& cats = boost::fusion::at_c<1> (Impl_->CategoryLinkInfo_->SingleFKeySelectors_) (naked.ID_);
			for (const auto& cat : cats)
				entry.Categories_ << Impl_->CatIDCache_ [cat.Category_].Name_;

			entries << entry;
		}

		return entries;
	}

	void Storage::InitializeTables ()
	{
		Util::DBLock lock (Impl_->DB_);
		lock.Init ();

		Impl_->AccountInfo_ = Util::oral::AdaptPtr<Account> (Impl_->DB_);
		Impl_->NakedExpenseEntryInfo_ = Util::oral::AdaptPtr<NakedExpenseEntry> (Impl_->DB_);
		Impl_->ReceiptEntryInfo_ = Util::oral::AdaptPtr<ReceiptEntry> (Impl_->DB_);
		Impl_->CategoryInfo_ = Util::oral::AdaptPtr<Category> (Impl_->DB_);
		Impl_->CategoryLinkInfo_ = Util::oral::AdaptPtr<CategoryLink> (Impl_->DB_);
		Impl_->RateInfo_ = Util::oral::AdaptPtr<Rate> (Impl_->DB_);

		lock.Good ();
	}

	void Storage::LoadCategories ()
	{
		for (const auto& cat : Impl_->CategoryInfo_->DoSelectAll_ ())
		{
			Impl_->CatCache_ [cat.Name_] = cat;
			Impl_->CatIDCache_ [cat.ID_] = cat;
		}
	}
}
}
