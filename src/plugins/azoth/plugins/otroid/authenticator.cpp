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

#include "authenticator.h"
#include <QInputDialog>
#include <QMessageBox>
#include <interfaces/azoth/iclentry.h>
#include "initiateauthdialog.h"

namespace LC
{
namespace Azoth
{
namespace OTRoid
{
	Authenticator::Authenticator (ICLEntry *entry)
	: QObject { entry->GetQObject () }
	, Entry_ { entry }
	, HrId_ { Entry_->GetHumanReadableID () }
	, Name_ { Entry_->GetEntryName () }
	{
	}

	Authenticator::~Authenticator ()
	{
		emit destroyingAuth (Entry_);
	}

	void Authenticator::AskFor (SmpMethod method, const QString& question, ConnContext *context)
	{
		QString str;
		switch (method)
		{
		case SmpMethod::Question:
			str = tr ("%1 (%2) wants to authenticate with you via a question. The question is:")
					.arg (Name_)
					.arg (HrId_);
			str += " <em>" + question + "</em>";
			break;
		case SmpMethod::SharedSecret:
			str = tr ("%1 (%2) wants to authenticate with you via a shared secret.")
					.arg (Name_)
					.arg (HrId_);
			break;
		}

		const auto& reply = QInputDialog::getText (nullptr,
				tr ("OTR authentication"),
				str);
		if (reply.isEmpty ())
		{
			emit abortSmp (context);
			deleteLater ();
			return;
		}

		emit gotReply (method, reply, context);
	}

	void Authenticator::Initiate ()
	{
		InitiateAuthDialog dia { Entry_ };
		if (dia.exec () != QDialog::Accepted)
		{
			deleteLater ();
			return;
		}

		emit initiateRequested (Entry_, dia.GetMethod (), dia.GetQuestion (), dia.GetAnswer ());
	}

	void Authenticator::Failed ()
	{
		QMessageBox::critical (nullptr,
				tr ("OTR authentication"),
				tr ("Failed to authenticate %1 (%2).")
						.arg ("<em>" + Name_ + "</em>")
						.arg ("<em>" + HrId_ + "</em>"));
	}

	void Authenticator::Cheated ()
	{
		QMessageBox::critical (nullptr,
				tr ("OTR authentication"),
				tr ("Failed to authenticate %1 (%2): cheating detected.")
						.arg ("<em>" + Name_ + "</em>")
						.arg ("<em>" + HrId_ + "</em>"));
	}

	void Authenticator::Success ()
	{
		QMessageBox::information (nullptr,
				tr ("OTR authentication"),
				tr ("Congratulations! Contact %1 (%2) authenticated successfully!")
						.arg ("<em>" + Name_ + "</em>")
						.arg ("<em>" + HrId_ + "</em>"));
	}
}
}
}
