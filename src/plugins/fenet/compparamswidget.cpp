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

#include "compparamswidget.h"
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QtDebug>
#include <util/compat/fontwidth.h>
#include <util/sll/curry.h>
#include "compparamsmanager.h"

namespace LC::Fenet
{
	namespace
	{
		class EditDelegate : public QStyledItemDelegate
		{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
			void setEditorData (QWidget*, const QModelIndex&) const override;
			void setModelData (QWidget*, QAbstractItemModel*, const QModelIndex&) const override;
		};

		QWidget* EditDelegate::createEditor (QWidget *parent,
				const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			const auto& var = index.data (CompParamsManager::Role::Description);
			if (!var.canConvert<Param> ())
				return QStyledItemDelegate::createEditor (parent, option, index);

			return new QDoubleSpinBox (parent);
		}

		void EditDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
		{
			const auto& var = index.data (CompParamsManager::Role::Description);
			if (!var.canConvert<Param> ())
				return QStyledItemDelegate::setEditorData (editor, index);

			const auto& param = var.value<Param> ();

			auto box = qobject_cast<QDoubleSpinBox*> (editor);
			box->setValue (index.data (Qt::EditRole).toDouble ());
			box->setMinimum (param.Min_);
			box->setMaximum (param.Max_);
		}

		void EditDelegate::setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const
		{
			const auto& var = index.data (CompParamsManager::Role::Description);
			if (!var.canConvert<Param> ())
				return QStyledItemDelegate::setModelData (editor, model, index);

			auto box = qobject_cast<QDoubleSpinBox*> (editor);
			const auto value = box->value ();

			model->setData (index, value);
		}
	}

	CompParamsWidget::CompParamsWidget (QWidget *parent)
	: QTreeView (parent)
	{
		setItemDelegate (new EditDelegate (this));
		setRootIsDecorated (false);

		auto width = Util::Curry (&Util::Compat::Width, fontMetrics ());
		setColumnWidth (0, width ("Average compositor option description length"));
		setColumnWidth (1, width ("average value"));
		setColumnWidth (2, width ("flag"));
	}

	void CompParamsWidget::accept ()
	{
		emit accepted ();
	}

	void CompParamsWidget::reject ()
	{
		emit rejected ();
	}
}
