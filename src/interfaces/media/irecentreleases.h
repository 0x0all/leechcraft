/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <QDateTime>
#include <QUrl>

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
		virtual void RequestRecentReleases (int number, bool withRecommends) = 0;

		/** @brief Returns the service name.
		 *
		 * This string returns a human-readable string with the service
		 * name, like "Last.FM".
		 *
		 * @return The human-readable service name.
		 */
		virtual QString GetServiceName () const = 0;
	protected:
		/** @brief Emitted when the list of recent releases is fetched.
		 *
		 * @param[out] releases The list of recent releases.
		 */
		virtual void gotRecentReleases (const QList<AlbumRelease>& releases) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IRecentReleases, "org.LeechCraft.Media.IRecentReleases/1.0");
