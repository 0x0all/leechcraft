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

#pragma once

#include <QString>

namespace LeechCraft
{
namespace Blogique
{
	class IPluginProxy
	{
	public:
		virtual ~IPluginProxy () {}

		/** @brief Retrieves the password for the given account.
		 *
		 * Returns password for the given account, or a null string if
		 * no password is stored. The password should be previously set
		 * (stored) by the call to SetPassword().
		 *
		 * The account object should implement the IAccount interface.
		 * Accounts are distinguished by their IDs.
		 *
		 * @param[in] account The account for which to retrieve the
		 * password. The object should implement IAccount.
		 * @return The stored password or null string if no password is
		 * stored.
		 *
		 * @note This function may return null passwords even after
		 * corresponding calls to SetPassword() since the implementation
		 * uses secure storage plugins to store passwords, and if no
		 * such plugins are installed, no passwords are stored.
		 *
		 * @sa GetAccountPassword(), SetPassword(), IAccount
		 */
		virtual QString GetPassword (QObject *account) = 0;

		/** @brief Stores the password for the given account.
		 *
		 * The password set by this function overwrites any previously
		 * set ones. After this function is called for given account,
		 * the GetPassword() would return either the last stored
		 * password or null string if no password storage plugin is
		 * installed.
		 *
		 * The account object should implement the IAccount interface.
		 * Accounts are distinguished by their IDs.
		 *
		 * @param[in] password The password string to store. Null string
		 * may be used to overwrite/clear the saved password.
		 * @param[in] account The account for which the password should
		 * be stored. The object should implement IAccount.
		 *
		 * @sa GetPassword(), IAccount
		 */
		virtual void SetPassword (const QString& password, QObject *account) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Blogique::IPluginProxy,
		"org.Deviant.LeechCraft.Plugins.Blogique.Plugins.IPluginProxy/1.0");

