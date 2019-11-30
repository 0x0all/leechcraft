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

#include "tocwidget.h"
#include <algorithm>
#include <iterator>
#include <QStandardItemModel>
#include <QtDebug>

namespace LC
{
namespace Monocle
{
	uint qHash (const LC::Monocle::ILink_ptr& link)
	{
		return ::qHash (link.get ());
	}

	TOCWidget::TOCWidget (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.TOCTree_->setModel (Model_);
	}

	namespace
	{
		auto Tuplize (const IPageLink_ptr& link)
		{
			return std::make_tuple (link->GetPageNumber (),
					link->NewX (),
					link->NewY ());
		}
	}

	void TOCWidget::SetTOC (const TOCEntryLevel_t& topLevel)
	{
		setEnabled (!topLevel.isEmpty ());

		Item2Link_.clear ();
		Link2Item_.clear ();
		IntraDocPageLinks_.clear ();
		Model_->clear ();

		AddWorker (Model_, topLevel);

		std::sort (IntraDocPageLinks_.begin (), IntraDocPageLinks_.end (),
				[] (const auto& left, const auto& right)
				{
					return Tuplize (left) < Tuplize (right);
				});

		Ui_.TOCTree_->expandToDepth (0);
	}

	namespace
	{
		QString NormalizeName (QString name)
		{
			return name
					.replace ("\r\n", "\n")
					.replace ("\r", "\n");
		}
	}

	template<typename T>
	void TOCWidget::AddWorker (T addable, const TOCEntryLevel_t& level)
	{
		for (const auto& entry : level)
		{
			const auto& name = NormalizeName (entry.Name_);

			auto item = new QStandardItem (QString { name }.replace ('\n', ' '));
			item->setToolTip (name);
			item->setEditable (false);
			Item2Link_ [item] = entry.Link_;
			Link2Item_ [entry.Link_] = item;

			AddWorker (item, entry.ChildLevel_);

			addable->appendRow (item);

			if (const auto ipl = std::dynamic_pointer_cast<IPageLink> (entry.Link_))
				if (ipl->GetDocumentFilename ().isEmpty ())
					IntraDocPageLinks_ << ipl;
		}
	}

	void TOCWidget::updateCurrentPage (int index)
	{
		const auto linkPos = std::upper_bound (IntraDocPageLinks_.begin (),
				IntraDocPageLinks_.end (),
				index,
				[] (int index, const auto& link) { return index < link->GetPageNumber (); });
		if (linkPos == IntraDocPageLinks_.begin ())
			return;

		const auto item = Link2Item_.value (std::dynamic_pointer_cast<ILink> (*std::prev (linkPos)));
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "no item for page"
					<< index;
			return;
		}

		Ui_.TOCTree_->setCurrentIndex (item->index ());
	}

	void TOCWidget::on_TOCTree__activated (const QModelIndex& index)
	{
		auto item = Model_->itemFromIndex (index);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid item for"
					<< index;
			return;
		}

		auto link = Item2Link_ [item];
		if (!link)
		{
			qWarning () << Q_FUNC_INFO
					<< "no link for item"
					<< item
					<< index;
			return;
		}

		link->Execute ();
	}
}
}
