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

#include "networkdiskcachegc.h"
#include <QTimer>
#include <QDir>
#include <QDirIterator>
#include <QDateTime>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <util/sll/futures.h>

namespace LeechCraft
{
namespace Util
{
	NetworkDiskCacheGC::NetworkDiskCacheGC ()
	{
		const auto timer = new QTimer { this };
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (handleCollect ()));
		timer->start (60 * 60 * 1000);
	}

	NetworkDiskCacheGC& NetworkDiskCacheGC::Instance ()
	{
		static NetworkDiskCacheGC gc;
		return gc;
	}

	std::shared_ptr<void> NetworkDiskCacheGC::RegisterDirectory (const QString& path, const std::function<int ()>& sizeGetter)
	{
		auto& list = Directories_ [path];
		list.push_front (sizeGetter);
		const auto thisItem = list.begin ();

		return std::shared_ptr<void>
		{
			nullptr,
			[this, path, thisItem] (void*) { UnregisterDirectory (path, thisItem); }
		};
	}

	void NetworkDiskCacheGC::UnregisterDirectory (const QString& path, CacheSizeGetters_t::iterator pos)
	{
		if (!Directories_.contains (path))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown directory"
					<< path;
			return;
		}

		auto& list = Directories_ [path];
		list.erase (pos);

		if (!list.isEmpty ())
			return;

		Directories_.remove (path);
	}

	namespace
	{
		qint64 Collector (const QString& cacheDirectory, qint64 goal)
		{
			if (cacheDirectory.isEmpty ())
				return 0;

			qDebug () << Q_FUNC_INFO << "running..." << cacheDirectory;

			const QDir::Filters filters = QDir::AllDirs | QDir:: Files | QDir::NoDotAndDotDot;
			QDirIterator it { cacheDirectory, filters, QDirIterator::Subdirectories };

			QMultiMap<QDateTime, QString> cacheItems;
			qint64 totalSize = 0;
			while (it.hasNext ())
			{
				const auto& path = it.next ();
				const auto& info = it.fileInfo ();
				cacheItems.insert (info.created (), path);
				totalSize += info.size ();
			}

			auto i = cacheItems.constBegin ();
			while (i != cacheItems.constEnd ())
			{
				if (totalSize < goal)
					break;

				QFile file { *i };
				const auto size = file.size ();
				totalSize -= size;
				++i;

				file.remove ();
			}

			qDebug () << "collector finished" << totalSize;

			return totalSize;
		}
	};

	void NetworkDiskCacheGC::handleCollect ()
	{
		if (IsCollecting_)
		{
			qWarning () << Q_FUNC_INFO
					<< "already collecting";
			return;
		}

		QList<QPair<QString, int>> dirs;
		for (const auto& pair : Util::Stlize (Directories_))
		{
			const auto& getters = pair.second;
			const auto minSize = (*std::min_element (getters.begin (), getters.end (),
						Util::ComparingBy (Apply))) ();
			dirs.append ({ pair.first, minSize });
		}

		if (dirs.isEmpty ())
			return;

		IsCollecting_ = true;

		Util::ExecuteFuture ([dirs]
				{
					return QtConcurrent::run ([dirs]
							{
								for (const auto& pair : dirs)
									Collector (pair.first, pair.second);
							});
				},
				[this] { IsCollecting_ = false; },
				this);
	}
}
}
