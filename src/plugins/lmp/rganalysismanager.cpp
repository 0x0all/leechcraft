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

#include "rganalysismanager.h"
#include "localcollection.h"
#include "localcollectionstorage.h"
#include "engine/rganalyser.h"

namespace LeechCraft
{
namespace LMP
{
	RgAnalysisManager::RgAnalysisManager (LocalCollection *coll, QObject *parent)
	: QObject { parent }
	, Coll_ { coll }
	{
		connect (Coll_,
				SIGNAL (scanFinished ()),
				this,
				SLOT (handleScanFinished ()));
	}

	void RgAnalysisManager::handleAnalysed ()
	{
		const auto& result = CurrentAnalyser_->GetResult ();
		CurrentAnalyser_.reset ();

		rotateQueue ();
	}

	void RgAnalysisManager::rotateQueue ()
	{
		if (AlbumsQueue_.isEmpty ())
			return;

		QStringList paths;
		for (const auto& track : AlbumsQueue_.takeFirst ()->Tracks_)
			paths << track.FilePath_;

		qDebug () << Q_FUNC_INFO << paths;
		CurrentAnalyser_.reset (new RgAnalyser (paths, this));
		connect (CurrentAnalyser_.get (),
				SIGNAL (finished ()),
				this,
				SLOT (handleAnalysed ()));
	}

	void RgAnalysisManager::handleScanFinished ()
	{
		QSet<int> albums;
		for (const auto track : Coll_->GetStorage ()->GetOutdatedRgTracks ())
			albums << Coll_->GetTrackAlbumId (track);

		const bool wasEmpty = AlbumsQueue_.isEmpty ();

		for (auto albumId : albums)
			AlbumsQueue_ << Coll_->GetAlbum (albumId);

		qDebug () << Q_FUNC_INFO
				<< AlbumsQueue_.size ()
				<< "albums to rescan";
		if (wasEmpty)
			rotateQueue ();
	}
}
}
