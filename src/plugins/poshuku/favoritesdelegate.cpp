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

#include "favoritesdelegate.h"
#include <util/tags/tagslineedit.h>
#include <util/tags/tagscompletionmodel.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "filtermodel.h"
#include "favoritesmodel.h"

namespace LC
{
namespace Poshuku
{
	using LC::Util::TagsCompleter;
	using LC::Util::TagsLineEdit;

	FavoritesDelegate::FavoritesDelegate (QObject *parent)
	: QItemDelegate (parent)
	{
	}

	QWidget* FavoritesDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem& opt, const QModelIndex& index) const
	{
		if (index.column () != FavoritesModel::ColumnTags)
			return QItemDelegate::createEditor (parent, opt, index);

		TagsLineEdit *tle = new TagsLineEdit (parent);
		TagsCompleter_.reset (new TagsCompleter (tle));
		tle->AddSelector ();
		return tle;
	}

	void FavoritesDelegate::setEditorData (QWidget *editor,
			const QModelIndex& index) const
	{
		if (index.column () != FavoritesModel::ColumnTags)
		{
			QItemDelegate::setEditorData (editor, index);
			return;
		}

		auto itm = Core::Instance ().GetProxy ()->GetTagsManager ();
		auto tags = Core::Instance ().GetFavoritesModel ()->data (index, RoleTags).toStringList ();
		static_cast<TagsLineEdit*> (editor)->setText (itm->Join (itm->GetTags (tags)));
	}

	void FavoritesDelegate::setModelData (QWidget *editor,
			QAbstractItemModel *model, const QModelIndex& index) const
	{
		if (index.column () != FavoritesModel::ColumnTags)
		{
			QItemDelegate::setModelData (editor, model, index);
			return;
		}

		auto tags = Core::Instance ().GetProxy ()->GetTagsManager ()->Split (static_cast<TagsLineEdit*> (editor)->text ());
		model->setData (index, tags);
	}

	void FavoritesDelegate::updateEditorGeometry (QWidget *editor,
			const QStyleOptionViewItem& option,
			const QModelIndex& index) const
	{
		if (index.column () != FavoritesModel::ColumnTags)
		{
			QItemDelegate::updateEditorGeometry (editor, option, index);
			return;
		}

		editor->setGeometry (option.rect);
	}
}
}
