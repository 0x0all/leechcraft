/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "pluginproxy.h"
#include <util/util.h>
#include "interfaces/blogique/iaccount.h"
#include "core.h"

namespace LeechCraft
{
namespace Blogique
{
	PluginProxy::PluginProxy (QObject*)
	{
	}

	void PluginProxy::SetPassword (const QString& password, QObject *accObj)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount"
					<< accObj;
			return;
		}

		QList<QVariant> keys;
		keys << "org.LeechCraft.Blogique.PassForAccount/" + acc->GetAccountID ();

		QList<QVariant> passwordVar;
		passwordVar << password;
		QList<QVariant> values;
		values << QVariant (passwordVar);

		Entity e = Util::MakeEntity (keys,
				QString (),
				Internal,
				"x-leechcraft/data-persistent-save");
		e.Additional_ ["Values"] = values;
		e.Additional_ ["Overwrite"] = true;

		Core::Instance ().SendEntity (e);
	}

	QString PluginProxy::GetPassword (QObject *accObj)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount"
					<< accObj;
			return QString ();
		}

		QList<QVariant> keys;
		keys << "org.LeechCraft.Blogique.PassForAccount/" + acc->GetAccountID ();
		const QVariantList& result = Util::GetPersistentData (keys,
				&Core::Instance ());
		if (result.size () != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect result size"
					<< result;
			return QString ();
		}

		const QVariantList& strVarList = result.at (0).toList ();
		if (strVarList.isEmpty () ||
				!strVarList.at (0).canConvert<QString> ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid string variant list"
					<< strVarList;
			return QString ();
		}

		return strVarList.at (0).toString ();
	}

}
}
