/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
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

#include "filesproxymodel.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "managertab.h"

namespace LC
{
namespace NetStoreManager
{
	FilesProxyModel::FilesProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
		setDynamicSortFilter (true);
		setFilterCaseSensitivity (Qt::CaseInsensitive);
		setSortCaseSensitivity (Qt::CaseInsensitive);
		setSortLocaleAware (true);
	}

	bool FilesProxyModel::filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
	{
		const QModelIndex& index = sourceModel ()->index (sourceRow, 0, sourceParent);
		return sourceModel ()->data (index).toString ().contains (filterRegExp ());
	}

	bool FilesProxyModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		bool leftIsFolder = sourceModel ()->data (left, ListingRole::IsDirectory).toBool ();
		bool rightIsFolder = sourceModel ()->data (right, ListingRole::IsDirectory).toBool ();

		if (left.column () == CName)
		{
			if (sourceModel ()->data (left, ListingRole::ID).toByteArray () == "netstoremanager.item_trash" ||
					sourceModel ()->data (right, ListingRole::ID).toByteArray () == "netstoremanager.item_uplevel")
				return sortOrder () == Qt::DescendingOrder ? true : false;
			if (sourceModel ()->data (right, ListingRole::ID).toByteArray () == "netstoremanager.item_trash" ||
					sourceModel ()->data (left, ListingRole::ID).toByteArray () == "netstoremanager.item_uplevel")
				return sortOrder () == Qt::DescendingOrder ? false : true;

			if (leftIsFolder && !rightIsFolder)
				return false;
			else if (!leftIsFolder && rightIsFolder)
				return true;
			else
				return QString::localeAwareCompare (sourceModel ()->data (left, SRName).toString ().toLower (),
						sourceModel ()->data (right, SRName).toString ().toLower ()) > 0;
		}
		else if (left.column () == CSize)
			return sourceModel ()->data (left, SRSize).toDouble () <
					sourceModel ()->data (right, SRSize).toDouble ();
		else if (left.column () == CModify)
			return sourceModel ()->data (left, SRModifyDate).toDateTime () <
					sourceModel ()->data (right, SRModifyDate).toDateTime ();
		else
			return QString::localeAwareCompare (sourceModel ()->data (left).toString ().toLower (),
					sourceModel ()->data (right).toString ().toLower ()) > 0;
	}

}
}
