/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItem>
#include "interfaces/blogique/iaccount.h"

namespace LC
{
namespace Blogique
{
namespace Utils
{
	enum EntriesViewColumns
	{
		Date,
		Subject
	};

	enum EntryIdRole
	{
		DBIdRole = Qt::UserRole + 1
	};

	QList<QStandardItem*> CreateEntriesViewRow (const Entry& entry);
}
}
}
