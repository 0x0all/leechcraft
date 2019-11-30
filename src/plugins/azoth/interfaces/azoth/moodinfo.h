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

#include <QString>

namespace LC
{
namespace Azoth
{
	/** @brief Describes contact mood information.
	 *
	 * It is modeled after XMPP XEP-0107, so please refer to
	 * http://xmpp.org/extensions/xep-0107.html for more information.
	 */
	struct MoodInfo
	{
		/** @brief Mood name as per XEP-0107.
		 *
		 * If this field is empty, the entry is considered to have stopped
		 * publishing mood information.
		 */
		QString Mood_;

		/** @brief Optional contact-set text accompanying the mood.
		 */
		QString Text_;
	};

	/** @brief Checks whether the mood info structures are equal.
	 *
	 * Returns true if \em i1 is equal to \em i2, containing the same
	 * values for all the fields, otherwise returns false.
	 *
	 * @param[in] i1 The first mood info structure.
	 * @param[in] i2 The second mood info structure.
	 * @return Whether \em i1 and \em i2 are equal.
	 */
	inline bool operator== (const MoodInfo& i1, const MoodInfo& i2)
	{
		return i1.Mood_ == i2.Mood_ &&
			   i1.Text_ == i2.Text_;
	}

	/** @brief Checks whether the mood info structures are not equal.
	 *
	 * Returns true if \em i1 is not equal to \em i2, that is, if at
	 * least one field of \em i1 is not equal to the corresponding one of
	 * \em i2. Otherwise returns false.
	 *
	 * @param[in] i1 The first mood info structure.
	 * @param[in] i2 The second mood info structure.
	 * @return Whether \em i1 and \em i2 are not equal.
	 */
	inline bool operator!= (const MoodInfo& i1, const MoodInfo& i2)
	{
		return !(i1 == i2);
	}
}
}
