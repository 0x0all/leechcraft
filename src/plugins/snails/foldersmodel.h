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
#include <array>
#include <boost/optional.hpp>
#include <QAbstractItemModel>
#include <QStringList>
#include "common.h"
#include "folder.h"

namespace LeechCraft
{
namespace Snails
{
	class Account;
	struct Folder;

	struct FolderDescr;
	typedef std::shared_ptr<FolderDescr> FolderDescr_ptr;

	class FoldersModel : public QAbstractItemModel
	{
		const QStringList Headers_;

		FolderDescr_ptr RootFolder_;
		QHash<QStringList, FolderDescr*> Folder2Descr_;

		std::array<QStringList, static_cast<int> (FolderType::Other)> CustomFolders_;
	public:
		enum Role
		{
			FolderPath = Qt::UserRole + 1
		};

		enum Column
		{
			FolderName,
			MessageCount
		};

		FoldersModel (Account*);

		QVariant headerData (int, Qt::Orientation, int) const override;
		int columnCount (const QModelIndex& = {}) const override;
		QVariant data (const QModelIndex&, int) const override;
		QModelIndex index (int, int, const QModelIndex& = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = {}) const override;

		void SetFolders (const QList<Folder>& folders);
		void SetFolderCounts (const QStringList&, int unread, int total);

		boost::optional<QStringList> GetFolderPath (FolderType) const;
	};
}
}
