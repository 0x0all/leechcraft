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

#include "historymessage.h"
#include <QtDebug>
#include <interfaces/azoth/imucentry.h>

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	HistoryMessage::HistoryMessage (IMessage::Direction dir,
			QObject *otherPart,
			Type type,
			const QString& variant,
			const QString& body,
			const QDateTime& dt,
			const QString& richBody,
			EscapePolicy policy)
	: Direction_ (dir)
	, OtherPart_ (otherPart)
	, Type_ (type)
	, Variant_ (variant)
	, Body_ (body)
	, DateTime_ (dt)
	, RichBody_ (richBody)
	, EscPolicy_ (policy)
	{
	}

	QObject* HistoryMessage::GetQObject ()
	{
		return this;
	}

	void HistoryMessage::Send ()
	{
		qWarning () << Q_FUNC_INFO
				<< "unable to send history message";
	}

	void HistoryMessage::Store ()
	{
		qWarning () << Q_FUNC_INFO
				<< "unable to store history message";
	}

	IMessage::Direction HistoryMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::Type HistoryMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::SubType HistoryMessage::GetMessageSubType () const
	{
		return SubType::Other;
	}

	QObject* HistoryMessage::OtherPart () const
	{
		return OtherPart_;
	}

	QString HistoryMessage::GetOtherVariant () const
	{
		return Variant_;
	}

	QString HistoryMessage::GetBody () const
	{
		return Body_;
	}

	void HistoryMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QDateTime HistoryMessage::GetDateTime () const
	{
		return DateTime_;
	}

	void HistoryMessage::SetDateTime (const QDateTime& dt)
	{
		DateTime_ = dt;
	}

	QString HistoryMessage::GetRichBody () const
	{
		return RichBody_;
	}

	void HistoryMessage::SetRichBody (const QString& body)
	{
		RichBody_ = body;
	}

	IMessage::EscapePolicy HistoryMessage::GetEscapePolicy () const
	{
		return EscPolicy_;
	}
}
}
}
