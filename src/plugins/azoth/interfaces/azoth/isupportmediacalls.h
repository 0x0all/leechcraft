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

#include <QtPlugin>

namespace LC
{
namespace Azoth
{
	/** @brief Interface for accounts supporting audio/video calls.
	 *
	 * This interface should be implemented by IAccount instances wishing
	 * to advertise the support for audio or video calls to the rest of
	 * Azoth.
	 *
	 * @sa IAccount
	 * @sa IMediaCall
	 */
	class ISupportMediaCalls
	{
	public:
		virtual ~ISupportMediaCalls () {}

		/** @brief Describes supported media call features.
		 */
		enum MediaCallFeature
		{
			/** @brief No particular features.
			 */
			MCFNoFeatures,

			/** @brief The account supports audio calls.
			 */
			MCFSupportsAudioCalls = 0x01,

			/** @brief The accounts supports video calls.
			 */
			MCFSupportsVideoCalls = 0x02
		};

		Q_DECLARE_FLAGS (MediaCallFeatures, MediaCallFeature)

		/** @brief Returns the media features supported by this account.
		 *
		 * @return The account media call features.
		 */
		virtual MediaCallFeatures GetMediaCallFeatures () const = 0;

		/** @brief Tries to call a contact list entry.
		 *
		 * The entry is identified by its \em id, which is the ID returned
		 * by ICLEntry::GetEntryID().
		 *
		 * If the corresponding protocol supports multiple variants per
		 * entry, the \em variant parameter specifies which variant (from
		 * the ones returned from ICLEntry::Variants()) of the entry
		 * should be called.
		 *
		 * Returns either a IMediaCall object or a nullptr if the call
		 * initialization failed.
		 *
		 * @note If this method returns a valid IMediaCall object, it
		 * should also be advertised via the called() signal.
		 *
		 * @param[in] id The ID of the entry to call (as in ICLEntry::GetEntryID()).
		 * @param[in] variant The variant of the entry to call to.
		 * @return The call object implementing IMediaCall, or a nullptr
		 * if it is impossible to make the call.
		 */
		virtual QObject* Call (const QString& id, const QString& variant) = 0;
	protected:
		/** @brief Emitted when a new call is established.
		 *
		 * This signal should be emitted whenever a new call is
		 * established, either an incoming call or an outgoing call
		 * initiated via the Call() method.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] callObj The object representing the call and
		 * implementing IMediaCall.
		 */
		virtual void called (QObject *callObj) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ISupportMediaCalls,
		"org.Deviant.LeechCraft.Azoth.ISupportMediaCalls/1.0")
