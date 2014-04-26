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

#include "annwidget.h"
#include <QMenu>
#include <QClipboard>
#include <util/sll/onetimerunner.h>
#include <interfaces/core/iiconthememanager.h>
#include "annmanager.h"
#include "anntreedelegate.h"
#include "core.h"

namespace LeechCraft
{
namespace Monocle
{
	AnnWidget::AnnWidget (AnnManager *mgr, QWidget *parent)
	: QWidget { parent }
	, Mgr_ { mgr }
	{
		Ui_.setupUi (this);

		Ui_.AnnTree_->setItemDelegate (new AnnTreeDelegate { Ui_.AnnTree_, this });
		Ui_.AnnTree_->setModel (Mgr_->GetModel ());

		connect (Mgr_,
				SIGNAL (annotationSelected (QModelIndex)),
				this,
				SLOT (focusOnAnnotation (QModelIndex)));
		connect (Ui_.AnnTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				Mgr_,
				SLOT (selectAnnotation (QModelIndex)));
	}

	void AnnWidget::on_AnnTree__customContextMenuRequested (const QPoint& point)
	{
		const auto& idx = Ui_.AnnTree_->indexAt (point);
		if (!idx.isValid () ||
				idx.data (AnnManager::Role::ItemType).toInt () == AnnManager::ItemTypes::PageItem)
			return;

		const auto itm = Core::Instance ().GetProxy ()->GetIconThemeManager ();

		QMenu menu;
		auto action = menu.addAction (itm->GetIcon ("edit-copy"), tr ("Copy annotation text"));
		new Util::OneTimeRunner
		{
			[&idx] () -> void
			{
				const auto& ann = idx.data (AnnManager::Role::Annotation).value<IAnnotation_ptr> ();
				qApp->clipboard ()->setText (ann->GetText ());
			},
			action,
			SIGNAL (triggered ()),
			&menu
		};

		menu.exec (Ui_.AnnTree_->viewport ()->mapToGlobal (point));
	}

	void AnnWidget::focusOnAnnotation (const QModelIndex& index)
	{
		QList<QModelIndex> expandList;
		auto parent = index.parent ();
		while (parent.isValid ())
		{
			expandList.prepend (parent);
			parent = parent.parent ();
		}

		for (const auto& idx : expandList)
			Ui_.AnnTree_->expand (idx);

		Ui_.AnnTree_->setCurrentIndex (index);
		Ui_.AnnTree_->selectionModel ()->select (index, QItemSelectionModel::SelectCurrent);
	}
}
}
