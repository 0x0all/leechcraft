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

#include "entriesdelegate.h"
#include <QSpinBox>
#include <QComboBox>
#include <QtDebug>
#include "localesmodel.h"
#include "util.h"

namespace LC
{
namespace Intermutko
{
	QWidget* EntriesDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem&, const QModelIndex& index) const
	{
		switch (static_cast<LocalesModel::Column> (index.column ()))
		{
		case LocalesModel::Column::Language:
		case LocalesModel::Column::Country:
			return new QComboBox { parent };
		case LocalesModel::Column::Quality:
		{
			const auto spinbox = new QDoubleSpinBox { parent };
			spinbox->setRange (0, 1);
			return spinbox;
		}
		case LocalesModel::Column::Code:
			return nullptr;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown column"
				<< index;
		return nullptr;
	}

	void EntriesDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
	{
		const auto& entryVar = index.data (static_cast<int> (LocalesModel::Role::LocaleEntry));
		const auto& entry = entryVar.value<LocaleEntry> ();

		switch (static_cast<LocalesModel::Column> (index.column ()))
		{
		case LocalesModel::Column::Language:
		{
			const auto combo = static_cast<QComboBox*> (editor);
			FillLanguageCombobox (combo);
			combo->setCurrentIndex (combo->findData (entry.Language_));
			break;
		}
		case LocalesModel::Column::Country:
		{
			const auto combo = static_cast<QComboBox*> (editor);
			FillCountryCombobox (combo, entry.Language_);
			combo->setCurrentIndex (combo->findData (entry.Country_));
			break;
		}
		case LocalesModel::Column::Quality:
			static_cast<QDoubleSpinBox*> (editor)->setValue (entry.Q_);
			break;
		case LocalesModel::Column::Code:
			break;
		}
	}

	void EntriesDelegate::setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const
	{
		switch (static_cast<LocalesModel::Column> (index.column ()))
		{
		case LocalesModel::Column::Language:
		case LocalesModel::Column::Country:
		{
			const auto combo = static_cast<QComboBox*> (editor);
			model->setData (index, combo->itemData (combo->currentIndex ()));
			break;
		}
		case LocalesModel::Column::Quality:
			model->setData (index, static_cast<QDoubleSpinBox*> (editor)->value ());
			break;
		case LocalesModel::Column::Code:
			break;
		}
	}
}
}
