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

#include "dataviewwidget.h"

namespace LC
{
	DataViewWidget::DataViewWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Ui_.Add_->setProperty ("ActionIcon", "list-add");
		Ui_.Modify_->setProperty ("ActionIcon", "configure");
		Ui_.Remove_->setProperty ("ActionIcon", "list-remove");

		setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

		connect (Ui_.Add_,
				SIGNAL (released ()),
				this,
				SIGNAL (addRequested ()));
		connect (Ui_.Modify_,
				SIGNAL (released ()),
				this,
				SIGNAL (modifyRequested ()));
		connect (Ui_.Remove_,
				SIGNAL (released ()),
				this,
				SIGNAL (removeRequested ()));
	}

	void DataViewWidget::DisableAddition ()
	{
		Ui_.Add_->setEnabled (false);
	}

	void DataViewWidget::DisableModification ()
	{
		Ui_.Modify_->setEnabled (false);
	}

	void DataViewWidget::DisableRemoval ()
	{
		Ui_.Remove_->setEnabled (false);
	}

	void DataViewWidget::AddCustomButton (const QByteArray& id, const QString& text)
	{
		auto button = new QPushButton (text);
		button->setProperty ("XSD/Id", id);
		Ui_.ButtonsLayout_->insertWidget (Ui_.ButtonsLayout_->count () - 1, button);

		connect (button,
				SIGNAL (released ()),
				this,
				SLOT (handleCustomButtonReleased ()));
	}

	void DataViewWidget::SetModel (QAbstractItemModel *model)
	{
		Ui_.View_->setModel (model);
	}

	QAbstractItemModel* DataViewWidget::GetModel () const
	{
		return Ui_.View_->model ();
	}

	QModelIndex DataViewWidget::GetCurrentIndex () const
	{
		return Ui_.View_->currentIndex ();
	}

	QModelIndexList DataViewWidget::GetSelectedRows () const
	{
		return Ui_.View_->selectionModel ()->selectedRows ();
	}

	void DataViewWidget::resizeColumns ()
	{
		Ui_.View_->expandAll ();
		for (auto i = 0; i < GetModel ()->columnCount (); ++i)
			Ui_.View_->resizeColumnToContents (i);
	}

	void DataViewWidget::handleCustomButtonReleased ()
	{
		auto button = qobject_cast<QPushButton*> (sender ());
		emit customButtonReleased (button->property ("XSD/Id").toByteArray ());
	}
}
