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

#include <memory>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <QObject>
#include <tox/toxav.h>
#include "threadexceptions.h"

template<typename T>
class QFuture;

namespace LeechCraft
{
namespace Azoth
{
namespace Sarin
{
	class ToxThread;

	template<typename... Errors>
	using MaybeError_t = boost::optional<boost::variant<Errors...>>;

	class CallManager : public QObject
	{
		Q_OBJECT

		ToxThread * const Thread_;
		std::unique_ptr<ToxAV, decltype (&toxav_kill)> ToxAv_;
	public:
		CallManager (ToxThread*, Tox*, QObject* = nullptr);

		using InitiateResult = MaybeError_t<UnknownFriendException, CallInitiateException>;
		QFuture<InitiateResult> InitiateCall (const QByteArray& pkey);

		struct WriteResult
		{
			qint64 Written_;
			QByteArray Leftover_;
		};
		QFuture<WriteResult> WriteData (int32_t callIdx, const QByteArray& data);

		using AcceptCallResult = MaybeError_t<CallAnswerException>;
		QFuture<AcceptCallResult> AcceptCall (int32_t callIdx);
	private:
		void HandleIncomingCall (int32_t callIdx);
		void HandleStateChanged (int32_t friendIdx, uint32_t state);

		void HandleAudio (int32_t call, const int16_t *frames, int size);
	signals:
		void gotIncomingCall (const QByteArray& pubkey, int32_t callIdx);
		void callStateChanged (int32_t callidx, uint32_t state);
		void gotFrame (int32_t call, const QByteArray&);
	};
}
}
}
