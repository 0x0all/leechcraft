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

#pragma once

#include <QObject>
#include <QFuture>
#include <util/threads/concurrentexception.h>
#include <tox/toxav.h>

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	using ToxException = Util::ConcurrentStdException;

	class TextExceptionBase
	{
		const QByteArray Msg_;
	public:
		TextExceptionBase (const QString& = {});

		const char* what () const noexcept;
	};

	using ThreadException = Util::ConcurrentException<Util::NewType<TextExceptionBase, NewTypeTag>>;
	using UnknownFriendException = Util::ConcurrentException<Util::NewType<TextExceptionBase, NewTypeTag>>;

	template<typename CodeType = int>
	class TypedCodeExceptionBase
	{
		const CodeType Code_;
		const QByteArray Msg_;
	public:
		TypedCodeExceptionBase (CodeType code)
		: Code_ { code }
		, Msg_ { QString { "Unable to perform action: %1." }.arg (code).toUtf8 () }
		{
		}

		TypedCodeExceptionBase (const QByteArray& context, CodeType code)
		: Code_ { code }
		, Msg_ { "Unable to perform action " + context + ": " + QByteArray::number (code) }
		{
		}

		const char* what () const noexcept
		{
			return Msg_;
		}

		CodeType GetCode () const noexcept
		{
			return Code_;
		}
	};

	using CallInitiateException = Util::ConcurrentException<Util::NewType<TypedCodeExceptionBase<TOXAV_ERR_CALL>, NewTypeTag>>;
	using FrameSendException = Util::ConcurrentException<Util::NewType<TypedCodeExceptionBase<TOXAV_ERR_SEND_FRAME>, NewTypeTag>>;
	using CallAnswerException = Util::ConcurrentException<Util::NewType<TypedCodeExceptionBase<TOXAV_ERR_ANSWER>, NewTypeTag>>;

	template<typename T>
	using CommandCodeException = Util::ConcurrentException<TypedCodeExceptionBase<T>>;

	template<typename T>
	CommandCodeException<T> MakeCommandCodeException (const QByteArray& msg, T error)
	{
		return { msg, error };
	}
}
}
}
