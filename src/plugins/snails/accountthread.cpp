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

#include "accountthread.h"
#include <QMutexLocker>
#include <QtDebug>
#include "account.h"
#include "accountthreadworker.h"
#include "core.h"

namespace LeechCraft
{
namespace Snails
{
	GenericExceptionWrapper::GenericExceptionWrapper (const std::exception_ptr& ptr)
	{
		if (!ptr)
		{
			Msg_ = "no exception information";
			return;
		}

		try
		{
			std::rethrow_exception (ptr);
		}
		catch (const std::exception& e)
		{
			Msg_ = std::string { "generic exception of type `" } + typeid (e).name () +
					"`: `" + e.what () + "`";
		}
	}

	const char* GenericExceptionWrapper::what () const noexcept
	{
		return Msg_.c_str ();
	}

	AccountThread::AccountThread (bool isListening, const QString& name,
			const CertList_t& certs, Account *parent)
	: A_ { parent }
	, IsListening_ { isListening }
	, Name_ { name }
	, Certs_ { certs }
	{
	}

	void AccountThread::run ()
	{
		AccountThreadWorker atw { IsListening_, Name_, Certs_, A_ };

		ConnectSignals (&atw);

		Util::SlotClosure<Util::NoDeletePolicy> rotator
		{
			[this, &atw] { RotateFuncs (&atw); },
			this,
			SIGNAL (rotateFuncs ()),
			nullptr
		};

		const auto shouldRotate = [&]
			{
				QMutexLocker locker { &FunctionsMutex_ };
				return !Functions_.isEmpty ();
			} ();

		if (shouldRotate)
			RotateFuncs (&atw);

		QThread::run ();
	}

	void AccountThread::ConnectSignals (AccountThreadWorker *atw)
	{
		connect (atw,
				SIGNAL (gotProgressListener (ProgressListener_g_ptr)),
				A_,
				SIGNAL (gotProgressListener (ProgressListener_g_ptr)));

		connect (atw,
				SIGNAL (folderSyncFinished (QStringList, QByteArray)),
				A_,
				SLOT (handleFolderSyncFinished (QStringList, QByteArray)));
	}

	void AccountThread::RotateFuncs (AccountThreadWorker *atw)
	{
		decltype (Functions_) funcs;

		{
			QMutexLocker locker { &FunctionsMutex_ };

			using std::swap;
			swap (funcs, Functions_);
		}

		for (const auto& func : funcs)
			func.Executor_ (atw);
	}
}
}
