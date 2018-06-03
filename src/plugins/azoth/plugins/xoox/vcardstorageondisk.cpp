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

#include "vcardstorageondisk.h"
#include <QDir>
#include <QSqlError>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/db/oral/oral.h>
#include <util/sys/paths.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	struct VCardStorageOnDisk::VCardRecord
	{
		Util::oral::PKey<QString, Util::oral::NoAutogen> JID_;
		QString VCardIq_;

		static QString ClassName ()
		{
			return "VCards";
		}
	};

	struct VCardStorageOnDisk::PhotoHashRecord
	{
		Util::oral::PKey<QString, Util::oral::NoAutogen> JID_;
		QByteArray Hash_;

		static QString ClassName ()
		{
			return "PhotoHashes";
		}
	};
}
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Azoth::Xoox::VCardStorageOnDisk::VCardRecord,
		JID_,
		VCardIq_)

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Azoth::Xoox::VCardStorageOnDisk::PhotoHashRecord,
		JID_,
		Hash_)

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	namespace sph = Util::oral::sph;

	VCardStorageOnDisk::VCardStorageOnDisk (QObject *parent)
	: QObject { parent }
	, DB_ { QSqlDatabase::addDatabase ("QSQLITE",
				Util::GenConnectionName ("org.LeechCraft.Azoth.Xoox.VCards")) }
	{
		const auto& cacheDir = Util::GetUserDir (Util::UserDir::Cache, "azoth/xoox");
		DB_.setDatabaseName (cacheDir.filePath ("vcards.db"));
		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		AdaptedVCards_ = Util::oral::AdaptPtr<VCardRecord> (DB_);
		AdaptedPhotoHashes_ = Util::oral::AdaptPtr<PhotoHashRecord> (DB_);
	}

	void VCardStorageOnDisk::SetVCard (const QString& jid, const QString& vcard)
	{
		AdaptedVCards_->Insert ({ jid, vcard }, Util::oral::InsertAction::Replace::PKey<VCardRecord>);
	}

	boost::optional<QString> VCardStorageOnDisk::GetVCard (const QString& jid) const
	{
		return AdaptedVCards_->SelectOne (sph::fields<&VCardRecord::VCardIq_>, sph::_0 == jid);
	}

	void VCardStorageOnDisk::SetVCardPhotoHash (const QString& jid, const QByteArray& hash)
	{
		AdaptedPhotoHashes_->Insert ({ jid, hash }, Util::oral::InsertAction::Replace::PKey<PhotoHashRecord>);
	}

	boost::optional<QByteArray> VCardStorageOnDisk::GetVCardPhotoHash (const QString& jid) const
	{
		return AdaptedPhotoHashes_->SelectOne (sph::fields<&PhotoHashRecord::Hash_>, sph::_0 == jid);
	}
}
}
}
