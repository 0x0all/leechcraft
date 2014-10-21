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

#include "shortcutsmanager.h"
#include <QSignalMapper>
#include <QTextEdit>
#include <util/util.h>
#include "abbrevsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Abbrev
{
	ShortcutsManager::ShortcutsManager (AbbrevsManager *abbrevs, QObject *parent)
	: QObject { parent }
	, Mapper_ { new QSignalMapper { this } }
	, Abbrevs_ { abbrevs }
	{
		connect (Mapper_,
				SIGNAL (mapped (QWidget*)),
				this,
				SLOT (handleActivated (QWidget*)));
	}

	void ShortcutsManager::HandleTab (QWidget *tab)
	{
		const auto shortcut = new QShortcut { tab };
		shortcut->setKey (Sequence_);
		connect (shortcut,
				SIGNAL (activated ()),
				Mapper_,
				SLOT (map ()));
		Mapper_->setMapping (shortcut, tab);
		Tab2SC_ [tab] = shortcut;

		connect (tab,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleDestroyed (QObject*)));
	}

	QMap<QString, ActionInfo> ShortcutsManager::GetActionInfo () const
	{
		return Util::MakeMap<QString, ActionInfo> ({
				{
					"org.LeechCraft.Azoth.Abbrev.Expand",
					{
						tr ("Expand abbreviations in current message edit text."),
						QKeySequence {},
						{}
					}
				},
			});
	}

	void ShortcutsManager::SetShortcut (const QString&, const QKeySequences_t& seqs)
	{
		Sequence_ = seqs.value (0);

		for (const auto sc : Tab2SC_)
			sc->setKey (Sequence_);
	}

	void ShortcutsManager::handleActivated (QWidget *tab)
	{
		QTextEdit *edit = nullptr;
		QMetaObject::invokeMethod (tab,
				"getMsgEdit",
				Q_RETURN_ARG (QTextEdit*, edit));
		if (!edit)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get message edit"
					<< tab;
			return;
		}

		const auto& text = edit->toPlainText ();
		const auto& processed = Abbrevs_->Process (text);
		if (text == processed)
			return;

		edit->setPlainText (processed);
		edit->moveCursor (QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
	}

	void ShortcutsManager::handleDestroyed (QObject *tabObj)
	{
		Tab2SC_.remove (static_cast<QWidget*> (tabObj));
	}
}
}
}
