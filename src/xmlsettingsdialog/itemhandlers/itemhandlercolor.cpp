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

#include "itemhandlercolor.h"
#include <QColor>
#include <QGridLayout>
#include <QLabel>
#include <QtDebug>
#include "../colorpicker.h"

namespace LeechCraft
{
	bool ItemHandlerColor::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "color";
	}

	void ItemHandlerColor::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		const QString& labelString = XSD_->GetLabel (item);
		QLabel *label = new QLabel (labelString);
		label->setWordWrap (false);

		ColorPicker *picker = new ColorPicker (labelString);
		picker->setObjectName (item.attribute ("property"));
		picker->SetCurrentColor (XSD_->GetValue (item).value<QColor> ());

		connect (picker,
				SIGNAL (currentColorChanged (QColor)),
				this,
				SLOT (updatePreferences ()));

		picker->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		int row = lay->rowCount ();
		lay->setColumnMinimumWidth (0, 10);
		lay->setColumnStretch (0, 1);
		lay->setColumnStretch (1, 10);
		lay->addWidget (label, row, 0);
		lay->addWidget (picker, row, 1, Qt::AlignLeft);
	}

	QVariant ItemHandlerColor::GetValue (const QDomElement& item, QVariant value) const
	{
		if (!value.canConvert<QColor> ()
				|| !value.value<QColor> ().isValid ())
			value = QColor (item.attribute ("default"));

		return value;
	}

	void ItemHandlerColor::SetValue (QWidget *widget, const QVariant& value) const
	{
		ColorPicker *colorPicker = qobject_cast<ColorPicker*> (widget);
		if (!colorPicker)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a ColorPicker"
				<< widget;
			return;
		}
		colorPicker->SetCurrentColor (value.value<QColor> ());
	}

	void ItemHandlerColor::UpdateValue (QDomElement& element, const QVariant& value) const
	{
		element.setAttribute ("default", value.value<QColor> ().name ());
	}

	QVariant ItemHandlerColor::GetObjectValue (QObject *object) const
	{
		ColorPicker *colorPicker = qobject_cast<ColorPicker*> (object);
		if (!colorPicker)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a ColorPicker"
				<< object;
			return QVariant ();
		}
		return colorPicker->GetCurrentColor ();
	}
};
