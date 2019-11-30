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

#include <QDateTime>
#include <util/sll/eitherfwd.h>
#include "audiostructs.h"

class QObject;

template<typename>
class QFuture;

namespace Media
{
	/** @brief Describes a single artist photo.
	 */
	struct ArtistImage
	{
		/** @brief The title of the image.
		 */
		QString Title_;

		/** @brief The author if the image.
		 */
		QString Author_;

		/** @brief The date and time the image was taken.
		 */
		QDateTime Date_;

		/** @brief URL of the thumbnail version of the image.
		 */
		QUrl Thumb_;

		/** @brief URL of the full version of the image.
		 */
		QUrl Full_;
	};

	/** @brief Information about artist biography.
	 *
	 * For now this structure only contains basic information about the
	 * artist, duplicating ArtistInfo. This may be changed/extended some
	 * time in the future, though.
	 *
	 * @sa ArtistInfo
	 */
	struct ArtistBio
	{
		/** @brief Basic information about this artist.
		 */
		ArtistInfo BasicInfo_;

		/** @brief Other images for this artist.
		 *
		 * This list will typically include posters, concerts photos and
		 * similar stuff.
		 */
		QList<ArtistImage> OtherImages_;
	};

	/** @brief Interface for plugins supporting fetching artist biography.
	 *
	 * Plugins that support fetching artist biography from the sources
	 * Last.FM should implement this interface.
	 */
	class Q_DECL_EXPORT IArtistBioFetcher
	{
	public:
		virtual ~IArtistBioFetcher () {}

		/** @brief The result of an artist biography search query.
		 *
		 * The result of an artist biography search query is either a string with a
		 * human-readable error text, or a ArtistBio object.
		 *
		 * @sa ArtistBio
		 */
		using Result_t = LC::Util::Either<QString, ArtistBio>;

		/** @brief Returns the service name.
		 *
		 * This string returns a human-readable string with the service
		 * name, like "Last.FM".
		 *
		 * @return The human-readable service name.
		 */
		virtual QString GetServiceName () const = 0;

		/** @brief Requests the biography of the given artist.
		 *
		 * This function initiates a search for artist biography and
		 * returns a future with the biography search result.
		 *
		 * @param[in] artist The artist name.
		 * @param[in] additionalImages Whether additional images for the
		 * ArtistBio::OtherImages_ field should be requested.
		 * @return The pending biography future.
		 */
		virtual QFuture<Result_t> RequestArtistBio (const QString& artist, bool additionalImages = true) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IArtistBioFetcher, "org.LeechCraft.Media.IArtistBioFetcher/1.0")
