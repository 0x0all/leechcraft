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

#include <QStandardItemModel>
#include <util/models/rolenamesmixin.h>
#include "qmlconfig.h"

namespace LC
{
namespace Util
{
	/** @brief A model to be used with UnhideListViewBase.
	 *
	 * This model just defines some roles used in various "unhide" lists
	 * and exposes them to QML.
	 *
	 * @sa UnhideListViewBase
	 *
	 * @ingroup QmlUtil
	 */
	class UTIL_QML_API UnhideListModel : public RoleNamesMixin<QStandardItemModel>
	{
	public:
		/** @brief Various unhide roles to be set by the rows of this
		 * model.
		 */
		enum Roles
		{
			/** @brief The unique ID of the item represented by this row.
			 *
			 * Exposed under the name \em itemClass to QML.
			 */
			ItemClass = Qt::UserRole + 1,

			/** @brief The name of the item represented by this row
			 *
			 * Exposed under the name \em itemName to QML.
			 */
			ItemName,

			/** @brief The description of the item represented by this row.
			 *
			 * Exposed under the name \em itemDescr to QML.
			 */
			ItemDescription,

			/** @brief The URL of the icon of the item represented by this
			 * row.
			 *
			 * Exposed under the name \em itemIcon to QML.
			 */
			ItemIcon
		};

		/** @brief Constructs the model with the given \em parent.
		 *
		 * @param[in] parent The parent object of this model.
		 */
		UnhideListModel (QObject *parent);
	};
}
}
