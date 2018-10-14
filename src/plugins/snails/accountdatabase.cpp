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

#include "accountdatabase.h"
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtDebug>
#include <util/sll/functor.h>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/db/oral/oral.h>
#include "account.h"

namespace LeechCraft
{
namespace Snails
{
	namespace oral = Util::oral;

	struct AccountDatabase::Message
	{
		oral::PKey<int> Id_;
		oral::Unique<QByteArray> UniqueId_;
		oral::NotNull<bool> IsRead_;

		static QString ClassName ()
		{
			return "Messages";
		}
	};

	struct AccountDatabase::Folder
	{
		oral::PKey<int> Id_;
		oral::Unique<Util::oral::NotNull<QString>> FolderPath_;

		static QString ClassName ()
		{
			return "Folders";
		}
	};

	struct AccountDatabase::Msg2Folder
	{
		oral::PKey<int> Id_;
		oral::References<&Message::Id_> MsgId_;
		oral::References<&Folder::Id_> FolderId_;
		oral::NotNull<QByteArray> FolderMessageId_;

		static QString ClassName ()
		{
			return "Msg2Folder";
		}
	};

	struct AccountDatabase::MsgHeader
	{
		oral::PKey<int> Id_;
		oral::References<&Message::UniqueId_> MsgUniqueId_;
		oral::NotNull<QByteArray> Header_;

		static QString ClassName ()
		{
			return "MsgHeader";
		}
	};
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Snails::AccountDatabase::Message,
		Id_,
		UniqueId_,
		IsRead_)

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Snails::AccountDatabase::Folder,
		Id_,
		FolderPath_)

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Snails::AccountDatabase::Msg2Folder,
		Id_,
		MsgId_,
		FolderId_,
		FolderMessageId_)

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Snails::AccountDatabase::MsgHeader,
		Id_,
		MsgUniqueId_,
		Header_)

namespace LeechCraft
{
namespace Snails
{
	AccountDatabase::AccountDatabase (const QDir& dir, const Account *acc)
	: DB_ { QSqlDatabase::addDatabase ("QSQLITE", "SnailsStorage_" + acc->GetID ()) }
	{
		DB_.setDatabaseName (dir.filePath ("msgs.db"));
		if (!DB_.open ())
		{
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error (qPrintable (QString ("Could not initialize database: %1")
						.arg (DB_.lastError ().text ())));
		}

		Util::RunTextQuery (DB_, "PRAGMA foreign_keys = ON;");
		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		Messages_ = Util::oral::AdaptPtr<Message> (DB_);
		Folders_ = Util::oral::AdaptPtr<Folder> (DB_);
		Msg2Folder_ = Util::oral::AdaptPtr<Msg2Folder> (DB_);
		MsgHeader_ = Util::oral::AdaptPtr<MsgHeader> (DB_);

		LoadKnownFolders ();
	}

	Util::DBLock AccountDatabase::BeginTransaction ()
	{
		Util::DBLock lock { DB_ };
		lock.Init ();
		return lock;
	}

	namespace sph = oral::sph;

	namespace
	{
		struct WithMessagesType {};
		struct WithoutMessagesType {};

		constexpr WithMessagesType WithMessages {};
		constexpr WithoutMessagesType WithoutMessages {};

		template<typename MessageTableSelector>
		auto FolderMessageIdSelector (const QByteArray& msgId, const QStringList& folder, MessageTableSelector)
		{
			using A = AccountDatabase;

			auto common = sph::f<&A::Folder::FolderPath_> == folder.join ("/") &&
					sph::f<&A::Msg2Folder::FolderMessageId_> == msgId &&
					sph::f<&A::Folder::Id_> == sph::f<&A::Msg2Folder::FolderId_>;

			if constexpr (std::is_same_v<MessageTableSelector, WithMessagesType>)
				return common &&
						sph::f<&A::Message::Id_> == sph::f<&A::Msg2Folder::MsgId_>;
			else
				return common;
		}
	}

	QList<QByteArray> AccountDatabase::GetIDs (const QStringList& folder)
	{
		return Msg2Folder_->Select (sph::fields<&Msg2Folder::FolderMessageId_>,
				sph::f<&Folder::FolderPath_> == folder.join ("/") &&
				sph::f<&Folder::Id_> == sph::f<&Msg2Folder::FolderId_>);
	}

	boost::optional<QByteArray> AccountDatabase::GetLastID (const QStringList& folder)
	{
		using Util::operator*;
		return [] (auto tup) { return std::get<0> (tup); } *
			Msg2Folder_->SelectOne.Build ()
				.Select (sph::fields<&Msg2Folder::FolderMessageId_, &Msg2Folder::Id_>)
				.Where (sph::f<&Folder::FolderPath_> == folder.join ("/") &&
						sph::f<&Folder::Id_> == sph::f<&Msg2Folder::FolderId_>)
				.Order (oral::OrderBy<sph::desc<&Msg2Folder::Id_>>)
				.Limit (1)
				();
	}

	int AccountDatabase::GetMessageCount (const QStringList& folder)
	{
		return Msg2Folder_->Select (sph::count<>,
				sph::f<&Folder::FolderPath_> == folder.join ("/") &&
				sph::f<&Folder::Id_> == sph::f<&Msg2Folder::FolderId_>);
	}

	int AccountDatabase::GetUnreadMessageCount (const QStringList& folder)
	{
		return Msg2Folder_->Select (sph::count<>,
				sph::f<&Folder::FolderPath_> == folder.join ("/") &&
				sph::f<&Folder::Id_> == sph::f<&Msg2Folder::FolderId_> &&
				sph::f<&Message::Id_> == sph::f<&Msg2Folder::MsgId_> &&
				sph::f<&Message::IsRead_> == false);
	}

	int AccountDatabase::GetMessageCount ()
	{
		return Messages_->Select (sph::count<>);
	}

	boost::optional<int> AccountDatabase::GetMsgTableId (const QByteArray& uniqueId)
	{
		if (uniqueId.isEmpty ())
			return {};

		return Messages_->SelectOne (sph::fields<&Message::Id_>,
				sph::f<&Message::UniqueId_> == uniqueId);
	}

	boost::optional<int> AccountDatabase::GetMsgTableId (const QByteArray& msgId, const QStringList& folder)
	{
		return Msg2Folder_->SelectOne (sph::fields<&Msg2Folder::MsgId_>,
				FolderMessageIdSelector (msgId, folder, WithoutMessages));
	}

	void AccountDatabase::AddMessage (const Message_ptr& msg)
	{
		for (const auto& folder : msg->GetFolders ())
			AddFolder (folder);

		Util::DBLock lock { DB_ };
		lock.Init ();

		for (const auto& folder : msg->GetFolders ())
		{
			if (GetMsgTableId (msg->GetFolderID (), folder))
			{
				qWarning () << Q_FUNC_INFO
						<< "skipping existing message"
						<< msg->GetFolderID ()
						<< "in folder"
						<< folder;
				continue;
			}

			const auto existing = GetMsgTableId (msg->GetMessageID ());
			const auto msgTableId = existing ?
					*existing :
					AddMessageUnfoldered (msg);
			AddMessageToFolder (msgTableId, GetFolder (folder), msg->GetFolderID ());
		}

		lock.Good ();
	}

	void AccountDatabase::RemoveMessage (const QByteArray& msgId, const QStringList& folder)
	{
		const auto id = Msg2Folder_->SelectOne (sph::fields<&Msg2Folder::Id_>,
				FolderMessageIdSelector (msgId, folder, WithoutMessages));
		if (id)
			Msg2Folder_->DeleteBy (sph::f<&Msg2Folder::Id_> == *id);
	}

	boost::optional<bool> AccountDatabase::IsMessageRead (const QByteArray& msgId, const QStringList& folder)
	{
		return Messages_->SelectOne (sph::fields<&Message::IsRead_>,
				FolderMessageIdSelector (msgId, folder, WithMessages));
	}

	void AccountDatabase::SetMessageRead (const QByteArray& msgId, const QStringList& folder, bool read)
	{
		auto msgTableId = GetMsgTableId (msgId, folder);
		if (!msgTableId)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown message"
					<< msgId
					<< "in folder"
					<< folder;
			return;
		}
		Messages_->Update (sph::f<&Message::IsRead_> = read,
				sph::f<&Message::Id_> == *msgTableId);
	}

	void AccountDatabase::SetMessageHeader (const QByteArray& msgId, const QByteArray& header)
	{
		MsgHeader_->Insert ({ {}, msgId, header }, oral::InsertAction::Replace::PKey<MsgHeader>);
	}

	boost::optional<QByteArray> AccountDatabase::GetMessageHeader (const QByteArray& uniqueMsgId) const
	{
		return MsgHeader_->SelectOne (sph::fields<&MsgHeader::Header_>,
				sph::f<&MsgHeader::MsgUniqueId_> == uniqueMsgId);
	}

	boost::optional<QByteArray> AccountDatabase::GetMessageHeader (const QStringList& folderId, const QByteArray& msgId) const
	{
		return MsgHeader_->SelectOne (sph::fields<&MsgHeader::Header_>,
		        FolderMessageIdSelector (msgId, folderId, WithMessages) &&
		        sph::f<&MsgHeader::MsgUniqueId_> == sph::f<&Message::UniqueId_>);
	}

	int AccountDatabase::AddMessageUnfoldered (const Message_ptr& msg)
	{
		return Messages_->Insert ({ {}, msg->GetMessageID (), msg->IsRead () });
	}

	void AccountDatabase::AddMessageToFolder (int msgTableId, int folderTableId, const QByteArray& msgId)
	{
		Msg2Folder_->Insert ({ {}, msgTableId, folderTableId, msgId });
	}

	int AccountDatabase::AddFolder (const QStringList& folder)
	{
		if (KnownFolders_.contains (folder))
			return KnownFolders_.value (folder);

		const auto id = Folders_->Insert ({ {}, { folder.join ("/") } });
		KnownFolders_ [folder] = id;
		return id;
	}

	int AccountDatabase::GetFolder (const QStringList& folder) const
	{
		if (!KnownFolders_.contains (folder))
			throw std::runtime_error ("Unknown folder");

		return KnownFolders_.value (folder);
	}

	void AccountDatabase::LoadKnownFolders ()
	{
		for (const auto& folder : Folders_->Select ())
			KnownFolders_ [(**folder.FolderPath_).split ('/')] = folder.Id_;
	}
}
}
