/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "itemhandlerlistview.h"
#include <QDomElement>
#include <QLabel>
#include <QListView>
#include <QGridLayout>
#include <QtDebug>
#include "../itemhandlerfactory.h"

namespace LC
{
	ItemHandlerListView::ItemHandlerListView (ItemHandlerFactory *factory, Util::XmlSettingsDialog *xsd)
	: ItemHandlerBase { xsd }
	, Factory_ { factory }
	{
	}

	bool ItemHandlerListView::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "listview";
	}

	void ItemHandlerListView::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());

		QListView *list = new QListView (XSD_->GetWidget ());

		const QString& prop = item.attribute ("property");
		list->setObjectName (prop);

		Factory_->RegisterDatasourceSetter (prop,
				[this] (const QString& str, QAbstractItemModel *m, Util::XmlSettingsDialog*)
					{ SetDataSource (str, m); });
		Propname2Listview_ [prop] = list;

		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		list->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		list->setProperty ("SearchTerms", label->text ());

		int row = lay->rowCount ();
		lay->addWidget (label, row, 0, Qt::AlignLeft);
		lay->addWidget (list, row + 1, 0);
	}

	QVariant ItemHandlerListView::GetValue (const QDomElement&, QVariant) const
	{
		return {};
	}

	void ItemHandlerListView::SetValue (QWidget*, const QVariant&) const
	{
	}

	void ItemHandlerListView::UpdateValue (QDomElement&, const QVariant&) const
	{
	}

	QVariant ItemHandlerListView::GetObjectValue (QObject*) const
	{
		return {};
	}

	void ItemHandlerListView::SetDataSource (const QString& prop, QAbstractItemModel *model)
	{
		QListView *list = Propname2Listview_ [prop];
		if (!list)
		{
			qWarning () << Q_FUNC_INFO
					<< "listview for property"
					<< prop
					<< "not found";
			return;
		}

		list->setModel (model);
	}
}
