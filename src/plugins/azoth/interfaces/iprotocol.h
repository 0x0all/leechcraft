/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_INTERFACES_IPROTOCOL_H
#define PLUGINS_AZOTH_INTERFACES_IPROTOCOL_H
#include <QFlags>
#include <QMetaType>

class QObject;

namespace LeechCraft
{
namespace Azoth
{
	class IAccount;
	class IProtocolPlugin;

	/** @brief Represents a protocol.
	 *
	 * IProtocol class represents a single protocol with its own set of
	 * accounts.
	 * 
	 * A protocol may also implement IURIHandler if it supports handling
	 * of various URIs, like xmpp: for XMPP protocol.
	 * 
	 * @sa IURIHandler
	 */
	class IProtocol
	{
	public:
		virtual ~IProtocol () {}

		enum ProtocolFeature
		{
			/** Multiuser chats are possible in this proto.
			 */
			PFSupportsMUCs,
			/** One could join MUCs as he wishes.
			 */
			PFMUCsJoinable
		};

		Q_DECLARE_FLAGS (ProtocolFeatures, ProtocolFeature);

		/** Returns the protocol object as a QObject.
		 *
		 * @return Protocol object as QObject.
		 */
		virtual QObject* GetObject () = 0;

		/** Returns the list of features supported by this protocol.
		 */
		virtual ProtocolFeatures GetFeatures () const = 0;

		/** Returns the accounts registered within this protocol.
		 *
		 * @return The list of accoutns of this protocol.
		 */
		virtual QList<QObject*> GetRegisteredAccounts () = 0;

		/** Returns the pointer to the parent protocol plugin that this
		 * protocol belongs to.
		 *
		 * @return The parent protocol plugin of this protocol.
		 */
		virtual QObject* GetParentProtocolPlugin () const = 0;

		/** Returns the human-readable name of this protocol, like
		 * "Jabber" or "ICQ".
		 *
		 * @return Human-readable name of the protocol.
		 */
		virtual QString GetProtocolName () const = 0;

		/** Returns the protocol ID, which must be unique among all the
		 * protocols.
		 *
		 * @return The unique ID of this protocol.
		 */
		virtual QByteArray GetProtocolID () const = 0;

		/** @brief Notifies the protocol that a new account should be
		 * registered.
		 *
		 * Protocol plugin is expected to ask the user for
		 * account details, register the account and emit
		 * the accountAdded(QObject*) signal.
		 */
		virtual void InitiateAccountRegistration () = 0;
		
		virtual QList<QWidget*> GetAccountRegistrationWidgets () = 0;
		virtual void RegisterAccount (const QString& name, const QList<QWidget*>& widgets) = 0;

		/** @brief Returns the widget used to set up the MUC join options.
		 *
		 * The returned widget should implement IMUCJoinWidget.
		 *
		 * The caller takes the ownership of the widget, so each time
		 * a newly constructed widget should be returned, and the plugin
		 * shouldn't delete the widget by itself.
		 *
		 * @sa IMUCJoinWidget
		 */
		virtual QWidget* GetMUCJoinWidget () = 0;
		
		/** @brief Returns the editor widget for the bookmarks of this
		 * protocol.
		 * 
		 * The returned widget must implement the
		 * IMUCBookmarkEditorWidget interface.
		 * 
		 * This function should create a new widget each time it is
		 * called, since the ownership is transferred to the caller and
		 * the widget will be deleted by the caller when appropriate.
		 * 
		 * @sa IMUCBookmarkEditorWidget
		 */
		virtual QWidget* GetMUCBookmarkEditorWidget () = 0;

		/** @brief Removes the given account.
		 *
		 * This function shouldn't ask anything from the user, just
		 * remove the account.
		 *
		 * If the account is not registered, this function should do
		 * nothing.
		 */
		virtual void RemoveAccount (QObject *account) = 0;

		virtual void accountAdded (QObject *account) = 0;
		virtual void accountRemoved (QObject *account) = 0;
	};

	Q_DECLARE_OPERATORS_FOR_FLAGS (IProtocol::ProtocolFeatures);
}
}

Q_DECLARE_METATYPE (LeechCraft::Azoth::IProtocol*);
Q_DECLARE_INTERFACE (LeechCraft::Azoth::IProtocol,
		"org.Deviant.LeechCraft.Azoth.IProtocol/1.0");

#endif
