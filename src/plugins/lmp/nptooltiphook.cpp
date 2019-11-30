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

#include "nptooltiphook.h"
#include <QEvent>
#include <QPixmap>
#include <QIcon>
#include <QToolTip>
#include <QPixmap>
#include <QHelpEvent>
#include <QFile>
#include <QtDebug>
#include <util/util.h>
#include "nowplayingpixmaphandler.h"
#include "core.h"
#include "localcollection.h"
#include "util.h"

namespace LC
{
namespace LMP
{
	NPTooltipHook::NPTooltipHook (NowPlayingPixmapHandler *handler, QObject *parent)
	: QObject { parent }
	, PxHandler_ { handler }
	{
		PxHandler_->AddSetter ([this] (const QPixmap&, const QString&) { Base64Px_.clear (); });
	}

	void NPTooltipHook::SetTrackInfo (const MediaInfo& info)
	{
		Info_ = info;
	}

	namespace
	{
		void SetStatistics (QString& str, const QString& path)
		{
			const auto& stats = Core::Instance ().GetLocalCollection ()->GetTrackStats (path);
			const auto& lastPlayStr = stats ?
					QObject::tr ("Last playback at %1")
						.arg (FormatDateTime (stats.LastPlay_)) :
					QString {};
			const auto& countStr = stats ?
					NPTooltipHook::tr ("Played %n time(s) since %1", 0, stats.Playcount_)
						.arg (FormatDateTime (stats.Added_)) :
					QString {};
			str.replace ("${PLAYBACKS}", countStr);
			str.replace ("${LASTPLAY}", lastPlayStr);
		}
	}

	bool NPTooltipHook::eventFilter (QObject *obj, QEvent *event)
	{
		if (event->type () != QEvent::ToolTip)
			return false;

		if (Base64Px_.isEmpty ())
		{
			const auto maxDim = 384;

			QImage img { PxHandler_->GetLastCoverPath () };
			if (img.isNull ())
				img = QIcon::fromTheme ("media-optical").pixmap (maxDim, maxDim).toImage ();

			if (img.width () > maxDim)
				img = img.scaled (maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);

			Base64Px_ = Util::GetAsBase64Src (img);
		}

		QFile file { ":/lmp/resources/templates/nptooltip.html" };
		if (!file.open (QIODevice::ReadOnly))
		{
			qCritical () << Q_FUNC_INFO
					<< "unable to open"
					<< file.fileName ()
					<< file.errorString ();
			return true;
		}

		auto str = QString::fromUtf8 (file.readAll ());
		str.replace ("${TITLE}", Info_.Title_);
		str.replace ("${ARTIST}", Info_.Artist_);
		str.replace ("${ALBUM}", Info_.Album_);
		str.replace ("${GENRE}", Info_.Genres_.join (" / "));
		SetStatistics (str, Info_.LocalPath_);
		str.replace ("${IMG}", Base64Px_);

		const auto he = static_cast<QHelpEvent*> (event);
		QToolTip::showText (he->globalPos (), str, static_cast<QWidget*> (obj));

		return true;
	}
}
}
