/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#ifndef Q_MOC_RUN
#include <boost/bimap.hpp>
#endif

#include <QObject>
#include <QStringList>
#include <QMultiMap>
#include "fileswatcherbase.h"

class QTimer;

namespace LC
{
namespace NetStoreManager
{
	class FilesWatcherInotify : public FilesWatcherBase
	{
		Q_OBJECT

		int INotifyDescriptor_;
		const uint32_t WatchMask_;
		const int  WaitMSecs_;
		size_t BufferLength_;
		size_t EventSize_;

		typedef boost::bimaps::bimap<QString, int> descriptorsMap;
		descriptorsMap WatchedPathes2Descriptors_;

		QStringList ExceptionMasks_;
		QTimer *Timer_;
	public:
		FilesWatcherInotify (QObject *parent = 0);
	private:
		void AddPath (const QString& path);
		void RenamePath (const QString& oldPath, const QString& newPath);
		void HandleNotification (int descriptor);
		void AddPathWithNotify (const QString& path);
		bool IsInExceptionList (const QString& path) const;
		void RemoveWatchingPath (int descriptor);
		void RemoveWatchingPath (const QString& path);

	public slots:
		void updatePaths (const QStringList& paths);

		void checkNotifications ();
		void release ();
		void updateExceptions (QStringList masks);
	};
}
}
