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

#include <QWidget>
#include "dbconfig.h"

namespace Ui
{
	class BackendSelector;
};

namespace LC
{
	namespace Util
	{
		class BaseSettingsManager;

		/** @brief A common dialog to select storage backend.
		 *
		 * Currently following backends are supported:
		 * - SQLite
		 * - PostgreSQL
		 *   If driver is not available, PostgreSQL will be grayed out.
		 *
		 * Communication is performed via BaseSettingsManager object
		 * passed to the constructor. The following properties are used
		 * in it:
		 * - StorageType of type QString
		 *   Could be either SQLite or PostgreSQL
		 * - SQLiteVacuum of type bool
		 *   Used if SQLite is chosen to set the VACUUM option.
		 * - SQLiteJournalMode of type string
		 *   Used if SQLite is chosen to set the desired journal mode.
		 * - SQLiteTempStore of type string
		 *   Used if SQLite is chosen to set the temporary storage.
		 * - SQLiteSynchronous of type string
		 *   Used if SQLite is chosen to set the sync mode.
		 * - PostgresHostname
		 *   Used if PostgreSQL is chosen to set server's host name.
		 * - PostgresPort
		 *   Used if PostgreSQL is chosen to set server's port.
		 * - PostgresDBName
		 *   Used if PostgreSQL is chosen to set database name.
		 * - PostgresUsername
		 *   Used if PostgreSQL is chosen to set user name for the
		 *   database.
		 * - PostgresPassword
		 *   Used if PostgreSQL is chosen to set password for the
		 *   database.
		 *
		 * These settings are also queried when constructing the
		 * selector to use them as default ones.
		 *
		 * @ingroup DbUtil
		 */
		class UTIL_DB_API BackendSelector : public QWidget
		{
			Q_OBJECT

			Ui::BackendSelector *Ui_;
			BaseSettingsManager *Manager_;
		public:
			/** @brief Constructs the BackendSelector.
			 *
			 * Constructs the BackendSelector from the given settings
			 * manager and parent widget.
			 *
			 * @param[in,out] manager The settings manager to use to
			 * communicate with the outer world.
			 * @param[in] parent The parent widget.
			 */
			BackendSelector (BaseSettingsManager *manager, QWidget *parent = 0);
		private:
			void FillUI ();
		public slots:
			/** @brief Updates the settings manager.
			 *
			 * This function fills the settings manager with the settings
			 * entered by the user in UI.
			 */
			void accept ();

			/** @brief Restores the settings in UI.
			 *
			 * This function restores the settings in UI from the
			 * settings in the settings manager.
			 */
			void reject ();
		};
	};
};
