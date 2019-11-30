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

#include "pixmapcachemanager.h"
#include <numeric>
#include <QtDebug>
#include "xmlsettingsmanager.h"
#include "pagegraphicsitem.h"

namespace LC
{
namespace Monocle
{
	PixmapCacheManager::PixmapCacheManager (QObject *parent)
	: QObject (parent)
	{
		XmlSettingsManager::Instance ().RegisterObject ("PixmapCacheSize",
				this, "handleCacheSizeChanged");
		handleCacheSizeChanged ();
	}

	namespace
	{
		quint64 GetPixmapSize (const QPixmap& px)
		{
			if (px.isNull ())
				return 0;

			return px.width () * px.height () * px.defaultDepth () / 8 * 1.5;
		}
	}

	void PixmapCacheManager::PixmapPainted (PageGraphicsItem *item)
	{
		RecentlyUsed_.removeAll (item);
		RecentlyUsed_ << item;
	}

	void PixmapCacheManager::PixmapChanged (PageGraphicsItem *item)
	{
		if (RecentlyUsed_.removeAll (item))
			CurrentSize_ = std::accumulate (RecentlyUsed_.begin (), RecentlyUsed_.end (), 0,
					[] (qint64 size, const PageGraphicsItem *item)
						{ return size + GetPixmapSize (item->pixmap ()); });

		RecentlyUsed_ << item;
		CurrentSize_ += GetPixmapSize (item->pixmap ());
		CheckCache ();
	}

	void PixmapCacheManager::PixmapDeleted (PageGraphicsItem *item)
	{
		CurrentSize_ -= GetPixmapSize (item->pixmap ());
		RecentlyUsed_.removeAll (item);
	}

	void PixmapCacheManager::CheckCache ()
	{
		for (auto i = RecentlyUsed_.begin (); i != RecentlyUsed_.end () && MaxSize_ < CurrentSize_; )
		{
			const auto page = *i;
			if (page->IsDisplayed ())
			{
				++i;
				continue;
			}

			const quint64 pxSize = GetPixmapSize (page->pixmap ());
			CurrentSize_ -= pxSize;
			page->ClearPixmap ();
			i = RecentlyUsed_.erase (i);
		}

		if (MaxSize_ < CurrentSize_)
			qWarning () << Q_FUNC_INFO
					<< "cache overflow:"
					<< CurrentSize_
					<< "instead of"
					<< MaxSize_
					<< "for"
					<< RecentlyUsed_.size ()
					<< "pages";
	}

	void PixmapCacheManager::handleCacheSizeChanged ()
	{
		MaxSize_ = XmlSettingsManager::Instance ().property ("PixmapCacheSize").value<qint64> () * 1024 * 1024;

		CheckCache ();
	}
}
}
