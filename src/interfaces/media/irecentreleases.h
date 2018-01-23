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
#include <QDateTime>
#include <QUrl>
#include <util/sll/eitherfwd.h>

template<typename>
class QFuture;

namespace Media
{
	/** @brief Describes a recent release.
	 *
	 * This structure is used to describe a recent release that may be of
	 * interest to the user.
	 *
	 * @sa IRecentReleases
	 */
	struct AlbumRelease
	{
		/** @brief The release title.
		 */
		QString Title_;

		/** @brief The artist performing this release.
		 */
		QString Artist_;

		/** @brief The date of this release.
		 */
		QDateTime Date_;

		/** @brief Thumbnail image of this release (like album art).
		 */
		QUrl ThumbImage_;

		/** @brief Full-size image of this release.
		 */
		QUrl FullImage_;

		/** @brief The page describing this release in more details.
		 */
		QUrl ReleaseURL_;
	};

	/** @brief Interface for plugins providing recent releases.
	 *
	 * This interface should be implemented by plugins providing
	 * information about recent releases based on user's musical taste
	 * (like Last.FM's service).
	 *
	 * Fetching recent releases is asynchronous in nature, so one should
	 * request the releases via the RequestRecentReleases() method and
	 * wait for the gotRecentReleases() signal.
	 */
	class Q_DECL_EXPORT IRecentReleases
	{
	public:
		virtual ~IRecentReleases () {}

		using Result_t = LeechCraft::Util::Either<QString, QList<AlbumRelease>>;

		/** @brief Requests the recent releases.
		 *
		 * If withRecommends is set to false then only releases by the
		 * artists in the user's library should be fetched. Otherwise,
		 * the result set may include (or consist only of) releases that
		 * are recommended to the user (based on his musical taste, for
		 * example) but aren't directly related to artists in his library.
		 *
		 * @param[in] number The number of releases to get.
		 * @param[in] withRecommends Whether recommendations or releases
		 * from user's library should be fetched.
		 */
		virtual QFuture<Result_t> RequestRecentReleases (int number, bool withRecommends) = 0;

		/** @brief Returns the service name.
		 *
		 * This string returns a human-readable string with the service
		 * name, like "Last.FM".
		 *
		 * @return The human-readable service name.
		 */
		virtual QString GetServiceName () const = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IRecentReleases, "org.LeechCraft.Media.IRecentReleases/1.0")
