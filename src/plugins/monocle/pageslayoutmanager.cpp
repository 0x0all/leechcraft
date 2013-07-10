/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "pageslayoutmanager.h"
#include <QGraphicsScene>
#include <QScrollBar>
#include <QTimer>
#include <QtDebug>
#include "interfaces/monocle/idynamicdocument.h"
#include "pagesview.h"
#include "pagegraphicsitem.h"
#include "common.h"

namespace LeechCraft
{
namespace Monocle
{
	const int Margin = 10;

	PagesLayoutManager::PagesLayoutManager (PagesView *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Scene_ (view->scene ())
	, LayMode_ (LayoutMode::OnePage)
	, ScaleMode_ (ScaleMode::FitWidth)
	, FixedScale_ (1)
	, RelayoutScheduled_ (false)
	, HorMargin_ (0)
	, VertMargin_ (0)
	, Rotation_ (0)
	{
		connect (View_,
				SIGNAL (sizeChanged ()),
				this,
				SLOT (scheduleRelayout ()),
				Qt::QueuedConnection);
	}

	void PagesLayoutManager::HandleDoc (IDocument_ptr doc, const QList<PageGraphicsItem*>& pages)
	{
		CurrentDoc_ = doc;
		Pages_ = pages;
		Rotation_ = 0;
		emit rotationUpdated (0);

		PageRotations_ = QVector<double> (pages.size (), 0);

		for (auto page : pages)
			page->SetLayoutManager (this);

		if (CurrentDoc_ && qobject_cast<IDynamicDocument*> (CurrentDoc_->GetQObject ()))
			connect (CurrentDoc_->GetQObject (),
					SIGNAL (pageSizeChanged (int)),
					this,
					SLOT (handlePageSizeChanged (int)));
	}

	const QList<PageGraphicsItem*>& PagesLayoutManager::GetPages () const
	{
		return Pages_;
	}

	LayoutMode PagesLayoutManager::GetLayoutMode () const
	{
		return LayMode_;
	}

	void PagesLayoutManager::SetLayoutMode (LayoutMode layMode)
	{
		if (layMode == LayMode_)
			return;

		LayMode_ = layMode;
	}

	int PagesLayoutManager::GetLayoutModeCount () const
	{
		return LayMode_ == LayoutMode::OnePage ? 1 : 2;
	}

	QPoint PagesLayoutManager::GetViewportCenter () const
	{
		const auto& rect = View_->viewport ()->contentsRect ();
		return QPoint (rect.width (), rect.height ()) / 2;
	}

	int PagesLayoutManager::GetCurrentPage() const
	{
		const auto& center = GetViewportCenter ();
		auto item = View_->itemAt (center - QPoint (1, 1));
		if (!item)
			item = View_->itemAt (center - QPoint (10, 10));
		auto pos = std::find_if (Pages_.begin (), Pages_.end (),
				[item] (decltype (Pages_.front ()) e) { return e == item; });
		return pos == Pages_.end () ? -1 : std::distance (Pages_.begin (), pos);
	}

	void PagesLayoutManager::SetCurrentPage (int idx, bool immediate)
	{
		if (idx < 0 || idx >= Pages_.size ())
			return;

		auto page = Pages_.at (idx);
		const auto& rect = page->boundingRect ();
		const auto& pos = page->scenePos ();
		int xCenter = pos.x () + rect.width () / 2;
		const auto visibleHeight = std::min (static_cast<int> (rect.height ()),
				View_->viewport ()->contentsRect ().height ());
		int yCenter = pos.y () + visibleHeight / 2;

		if (immediate)
			View_->centerOn (xCenter, yCenter);
		else
			View_->SmoothCenterOn (xCenter, yCenter);
	}

	void PagesLayoutManager::SetScaleMode (ScaleMode mode)
	{
		ScaleMode_ = mode;
	}

	ScaleMode PagesLayoutManager::GetScaleMode () const
	{
		return ScaleMode_;
	}

	void PagesLayoutManager::SetFixedScale (double scale)
	{
		FixedScale_ = scale;
	}

	double PagesLayoutManager::GetCurrentScale () const
	{
		if (!CurrentDoc_)
			return 1;

		auto calcRatio = [this] (std::function<double (const QSize&)> dimGetter) -> double
		{
			if (Pages_.isEmpty ())
				return 1.0;
			int pageIdx = GetCurrentPage ();
			if (pageIdx < 0)
				pageIdx = 0;

			double dim = dimGetter (GetRotatedSize (pageIdx).toSize () + QSize (2 * HorMargin_, 2 * VertMargin_));
			auto size = View_->maximumViewportSize ();
			size.rwidth () -= View_->verticalScrollBar ()->size ().width ();
			size.rheight () -= View_->horizontalScrollBar ()->size ().height ();

			const int margin = 3;
			size.rwidth () -= 2 * margin;
			size.rheight () -= 2 * margin;

			const auto res = dimGetter (size) / dim;
			return res > 0 ? res : 1;
		};

		switch (ScaleMode_)
		{
		case ScaleMode::FitWidth:
		{
			auto ratio = calcRatio ([] (const QSize& size) { return size.width (); });
			if (LayMode_ != LayoutMode::OnePage)
				ratio /= 2;
			return ratio;
		}
		case ScaleMode::FitPage:
		{
			auto wRatio = calcRatio ([] (const QSize& size) { return size.width (); });
			if (LayMode_ != LayoutMode::OnePage)
				wRatio /= 2;
			auto hRatio = calcRatio ([] (const QSize& size) { return size.height (); });
			return std::min (wRatio, hRatio);
		}
		case ScaleMode::Fixed:
			return FixedScale_;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown scale mode"
				<< static_cast<int> (ScaleMode_);
		return 1;
	}

	void PagesLayoutManager::SetRotation (double angle)
	{
		Rotation_ = angle;
		Relayout ();
		emit rotationUpdated (angle);
	}

	void PagesLayoutManager::AddRotation (double dAngle)
	{
		SetRotation (GetRotation () + dAngle);
	}

	double PagesLayoutManager::GetRotation () const
	{
		return Rotation_;
	}

	void PagesLayoutManager::SetRotation (double angle, int page)
	{
		PageRotations_ [page] = angle;
		Relayout ();
		emit rotationUpdated (angle, page);
	}

	void PagesLayoutManager::AddRotation (double dAngle, int page)
	{
		SetRotation (dAngle + GetRotation (page), page);
	}

	double PagesLayoutManager::GetRotation (int page) const
	{
		return PageRotations_ [page];
	}

	void PagesLayoutManager::SetMargins (double horizontal, double vertical)
	{
		HorMargin_ = horizontal;
		VertMargin_ = vertical;
	}

	void PagesLayoutManager::Relayout ()
	{
		const auto scale = GetCurrentScale ();
		const auto pageWas = GetCurrentPage ();
		const auto pageObj = Pages_.value (pageWas);

		QPointF oldPageCenter;
		if (pageWas >= 0)
		{
			const auto& pagePos = pageObj->mapFromScene (View_->mapToScene (GetViewportCenter ()));
			const auto& bounding = pageObj->boundingRect ();
			if (bounding.width () && bounding.height ())
				oldPageCenter = QPointF { pagePos.x () / bounding.width (),
						pagePos.y () / bounding.height () };
		}

		for (auto item : Pages_)
		{
			item->resetTransform ();

			const auto pageRotation = PageRotations_ [item->GetPageNum ()] + Rotation_;
			const auto& bounding = item->boundingRect ();
			item->setTransformOriginPoint (bounding.width () / 2, bounding.height () / 2);
			item->setRotation (pageRotation);

			item->SetScale (scale, scale);
		}

		for (int i = 0, pagesCount = Pages_.size (); i < pagesCount; ++i)
		{
			auto page = Pages_ [i];

			const auto& size = GetRotatedSize (i) * scale;

			switch (LayMode_)
			{
			case LayoutMode::OnePage:
				page->setPos (0, Margin + (size.height () + Margin) * i);
				break;
			case LayoutMode::TwoPages:
				page->setPos ((i % 2) * (Margin / 3 + size.width ()),
						Margin + (size.height () + Margin) * (i / 2));
				break;
			}
		}

		Scene_->setSceneRect (Scene_->itemsBoundingRect ()
					.adjusted (-HorMargin_, -VertMargin_, 0, 0));

		SetCurrentPage (std::max (pageWas, 0), true);
		if (pageWas >= 0)
		{
			const auto& bounding = pageObj->boundingRect ();
			const QPointF newCenter { bounding.width () * oldPageCenter.x (),
					bounding.height () * oldPageCenter.y () };
			View_->centerOn (pageObj->mapToScene (newCenter));
		}

		if (RelayoutScheduled_)
		{
			RelayoutScheduled_ = false;
			emit scheduledRelayoutFinished ();
		}
	}

	QSizeF PagesLayoutManager::GetRotatedSize (int page) const
	{
		const auto& origSize = CurrentDoc_->GetPageSize (page);
		return Pages_.at (page)->transform ().mapRect (QRectF { { 0, 0 }, origSize }).size ();
	}

	void PagesLayoutManager::scheduleSetRotation (double angle)
	{
		SetRotation (angle);
		emit rotationUpdated (angle);
	}

	void PagesLayoutManager::scheduleRelayout ()
	{
		if (RelayoutScheduled_)
			return;

		QTimer::singleShot (500,
				this,
				SLOT (handleRelayout ()));
		RelayoutScheduled_ = true;
	}

	void PagesLayoutManager::handleRelayout ()
	{
		if (!RelayoutScheduled_)
			return;

		Relayout ();
		emit scheduledRelayoutFinished ();
	}

	void PagesLayoutManager::handlePageSizeChanged (int)
	{
		scheduleRelayout ();
	}
}
}
