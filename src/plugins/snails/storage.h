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

#include <memory>
#include <optional>
#include <QObject>
#include <QDir>
#include <QSettings>
#include <QHash>
#include <QSet>

namespace LC
{
namespace Snails
{
	class Account;

	class AccountDatabase;
	using AccountDatabase_ptr = std::shared_ptr<AccountDatabase>;

	struct MessageInfo;
	struct MessageBodies;

	class Storage : public QObject
	{
		QDir SDir_;

		QHash<const Account*, AccountDatabase_ptr> AccountBases_;
		const Qt::HANDLE CachedThread_;
	public:
		Storage (QObject* = nullptr);

		AccountDatabase_ptr BaseForAccount (const Account*);

		void SaveMessageInfos (Account*, const QList<MessageInfo>&);
		QList<MessageInfo> GetMessageInfos (Account*, const QStringList& folder);
		std::optional<MessageInfo> GetMessageInfo (Account*, const QStringList& folder, const QByteArray& msgId);

		void SaveMessageBodies (Account*, const QStringList& folder, const QByteArray& msgId, const MessageBodies&);
		std::optional<MessageBodies> GetMessageBodies (Account*, const QStringList& folder, const QByteArray& msgId);
		bool HasMessageBodies (Account*, const QStringList& folder, const QByteArray& msgId);

		QList<QByteArray> LoadIDs (Account*, const QStringList& folder);
		std::optional<QByteArray> GetLastID (Account*, const QStringList& folder);
		void RemoveMessage (Account*, const QStringList&, const QByteArray&);

		int GetNumMessages (Account*, const QStringList& folder);
		int GetNumUnread (Account*, const QStringList& folder);

		bool IsMessageRead (Account*, const QStringList& folder, const QByteArray&);
		void SetMessagesRead (Account*, const QStringList& folder, const QList<QByteArray>& folderIds, bool read);
	private:
		QDir DirForAccount (const Account*) const;
	};
}
}
