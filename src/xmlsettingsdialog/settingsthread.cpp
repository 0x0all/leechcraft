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

#include "settingsthread.h"
#include <QMutexLocker>
#include <QTimer>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "basesettingsmanager.h"

namespace LeechCraft
{
	SettingsThread::SettingsThread (QObject *parent)
	: QObject (parent)
	{
	}

	SettingsThread::~SettingsThread ()
	{
		QMutexLocker l (&Mutex_);
		if (!Pendings_.isEmpty ())
			qWarning () << Q_FUNC_INFO
					<< "there are pending settings to be saved, unfortunately they will be lost :(";
	}

	void SettingsThread::Save (Util::BaseSettingsManager *bsm, QString name, QVariant value)
	{
		QMutexLocker l (&Mutex_);

		if (Pendings_.isEmpty ())
			QTimer::singleShot (0, this, SLOT (saveScheduled ()));
		Pendings_ [bsm].push_back ({ name, value });
	}

	void SettingsThread::saveScheduled ()
	{
		decltype (Pendings_) pendings;

		{
			QMutexLocker l (&Mutex_);
			using std::swap;
			swap (pendings, Pendings_);
		}

		for (const auto& pair : Util::Stlize (pendings))
		{
			const auto& s = pair.first->GetSettings ();
			for (const auto& p : pair.second)
				s->setValue (p.first, p.second);
		}
	}
}
