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

#include "modelitem.h"
#include <algorithm>

namespace LC
{
namespace Util
{
	ModelItem::ModelItem (QAbstractItemModel *model, const QModelIndex& idx, const ModelItem_wtr& parent)
	: ModelItemBase { parent }
	, Model_ { model }
	, SrcIdx_ { idx }
	{
	}

	ModelItem* ModelItem::EnsureChild (int row)
	{
		if (Children_.value (row))
			return Children_.at (row).get ();

		if (Children_.size () <= row)
			Children_.resize (row + 1);

		const auto& childIdx = Model_->index (row, 0, SrcIdx_);
		Children_ [row] = std::make_shared<ModelItem> (Model_, childIdx, shared_from_this ());
		return Children_.at (row).get ();
	}

	const QModelIndex& ModelItem::GetIndex () const
	{
		return SrcIdx_;
	}

	void ModelItem::RefreshIndex (int modelStartingRow)
	{
		if (SrcIdx_.isValid ())
			SrcIdx_ = Model_->index (GetRow () - modelStartingRow, 0, Parent_.lock ()->GetIndex ());
	}

	QAbstractItemModel* ModelItem::GetModel () const
	{
		return Model_;
	}

	ModelItem_ptr ModelItem::FindChild (QModelIndex index) const
	{
		index = index.sibling (index.row (), 0);

		const auto pos = std::find_if (Children_.begin (), Children_.end (),
				[&index] (const ModelItem_ptr& item) { return item->GetIndex () == index; });
		return pos == Children_.end () ? ModelItem_ptr {} : *pos;
	}
}
}
