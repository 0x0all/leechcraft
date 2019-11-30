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
  DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "messagelisteditormanager.h"
#include <QTreeView>
#include <util/sll/lambdaeventfilter.h>
#include "common.h"
#include "mailmodel.h"

namespace LC
{
namespace Snails
{
	MessageListEditorManager::MessageListEditorManager (QTreeView *view, QObject *parent)
	: QObject { parent }
	, View_ { view }
	, Mode_ { MailListMode::Normal }
	{
		View_->setMouseTracking (true);

		connect (View_,
				&QTreeView::entered,
				[this] (const QModelIndex& index)
				{
					CloseCurrent ();

					View_->openPersistentEditor (index);
					LastEdited_ = index;
				});

		const auto ef = Util::MakeLambdaEventFilter ([this] (QEvent *event)
				{
					if (event->type () == QEvent::Leave)
						CloseCurrent ();
					return false;
				},
				this);
		View_->installEventFilter (ef);
	}

	void MessageListEditorManager::SetMailListMode (MailListMode mode)
	{
		if (mode == Mode_)
			return;

		Mode_ = mode;
		HandleMessageListUpdated ();
	}

	void MessageListEditorManager::HandleMessageListUpdated ()
	{
		if (Mode_ == MailListMode::MultiSelect)
			CloseCurrent ();
	}

	void MessageListEditorManager::CloseCurrent ()
	{
		if (LastEdited_.isValid ())
		{
			View_->closePersistentEditor (LastEdited_);
			LastEdited_ = {};
		}
	}
}
}
