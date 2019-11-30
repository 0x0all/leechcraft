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

#include "application.h"
#include <thread>
#include <chrono>
#include <QFont>
#include <QSysInfo>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QtDebug>

#if defined(Q_OS_MAC) && !defined(USE_UNIX_LAYOUT)
#include <mach-o/dyld.h>

#include "util/sys/util.h"

namespace
{
	void SetupLibraryPaths ()
	{
		if (!LC::Util::IsOSXLoadFromBundle ())
			return;

		char path [1024] = { 0 };
		uint32_t pathLength = sizeof (path);
		if (const auto rc = _NSGetExecutablePath (path, &pathLength))
		{
			qCritical () << Q_FUNC_INFO
					<< "cannot get executable path:"
					<< rc;
			return;
		}

		auto dir = QFileInfo { path }.dir ();
		dir.cdUp ();
		dir.cd ("PlugIns");
		qDebug () << Q_FUNC_INFO
				<< "setting"
				<< dir.absolutePath ();
		QCoreApplication::setLibraryPaths ({ dir.absolutePath () });
	}
}
#endif

namespace
{
	void CheckDelay ()
	{
		const auto& delayVar = qgetenv ("LC_STARTUP_DELAY");
		if (delayVar.isEmpty ())
			return;

		bool ok = false;
		const auto delayVal = delayVar.toInt (&ok);
		if (!ok)
			return;

		std::this_thread::sleep_for (std::chrono::seconds (delayVal));
	}
}

int main (int argc, char **argv)
{
	int author = 0xd34df00d;
	Q_UNUSED (author);

	CheckDelay ();

#if defined(Q_OS_MAC) && !defined(USE_UNIX_LAYOUT)
	SetupLibraryPaths ();
#endif

#ifndef Q_OS_MAC
	QCoreApplication::setAttribute (Qt::AA_X11InitThreads);
#endif

	QCoreApplication::setAttribute (Qt::AA_ShareOpenGLContexts);
	QCoreApplication::setAttribute (Qt::AA_UseHighDpiPixmaps);

	LC::Application app (argc, argv);
	return app.exec ();
}

