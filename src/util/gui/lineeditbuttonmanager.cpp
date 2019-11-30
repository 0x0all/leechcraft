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

#include "lineeditbuttonmanager.h"
#include <stdexcept>
#include <QLineEdit>
#include <QStyle>
#include <QToolButton>
#include <QEvent>
#include <QtDebug>

namespace LC
{
namespace Util
{
	LineEditButtonManager::LineEditButtonManager (QLineEdit *edit)
	: QObject { edit }
	, Edit_ { edit }
	, FrameWidth_ { edit->style ()->pixelMetric (QStyle::PM_DefaultFrameWidth) }
	, Pad_ { 1 + FrameWidth_ }
	{
		edit->installEventFilter (this);

		if (edit->findChildren<LineEditButtonManager*> ().size () > 1)
		{
			std::string str { "LineEditButtonManager is already installed on the edit" };

			const auto& name = edit->objectName ();
			if (!name.isEmpty ())
				str += " " + name.toStdString ();

			throw std::runtime_error (str);
		}
	}

	void LineEditButtonManager::Add (QToolButton *button)
	{
		Buttons_ << button;

		const auto& buttonSH = button->sizeHint ();
		Pad_ += buttonSH.width ();

		Edit_->setStyleSheet (QString ("QLineEdit { padding-right: %1px; }")
					.arg (Pad_));

		UpdatePos ();
	}

	bool LineEditButtonManager::eventFilter (QObject *obj, QEvent *event)
	{
		if (event->type () == QEvent::Resize ||
			event->type () == QEvent::Move)
			UpdatePos ();

		return QObject::eventFilter (obj, event);
	}

	void LineEditButtonManager::UpdatePos ()
	{
		int sumWidth = 0;

		for (const auto button : Buttons_)
		{
			const auto& hint = button->sizeHint ();

			sumWidth += hint.width ();

			const auto& rect = Edit_->rect ();
			const int frameWidth = Edit_->style ()->pixelMetric (QStyle::PM_DefaultFrameWidth);
			button->move (rect.right () - frameWidth - sumWidth,
					(rect.bottom () + 1 - hint.height ()) / 2);
		}
	}
}
}
