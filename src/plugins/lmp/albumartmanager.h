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

#include <QObject>
#include <QDir>
#include <interfaces/media/ialbumartprovider.h>
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	class AlbumArtManager : public QObject
	{
		Q_OBJECT

		QDir AADir_;

		struct TaskQueue
		{
			Media::AlbumInfo Info_;
			bool PreviewMode_;
		};
		QList<TaskQueue> Queue_;
		QHash<Media::AlbumInfo, int> NumRequests_;

		QHash<Media::AlbumInfo, QSize> BestSizes_;
	public:
		AlbumArtManager (QObject*);

		void CheckAlbumArt (const Collection::Artist&, Collection::Album_ptr);
		void CheckAlbumArt (const QString& artist, const QString& album, bool preview);

		void HandleGotAlbumArt (const Media::AlbumInfo&, const QList<QImage>&);
	private:
		void HandleGotUrls (const TaskQueue&, const QList<QUrl>&);
	private slots:
		void rotateQueue ();

		void handleCoversPath ();
	signals:
		void gotImages (const Media::AlbumInfo&, const QList<QImage>&);
	};
}
}
