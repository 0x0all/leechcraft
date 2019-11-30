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
#include <QtPlugin>

template<typename>
class QList;

class QSslError;

namespace LC
{
namespace Azoth
{
	class ICanHaveSslErrors
	{
	protected:
		virtual ~ICanHaveSslErrors () = default;
	public:
		class ISslErrorsReaction
		{
		protected:
			virtual ~ISslErrorsReaction () = default;
		public:
			virtual void Ignore () = 0;
			virtual void Abort () = 0;
		};

		using ISslErrorsReaction_ptr = std::shared_ptr<ISslErrorsReaction>;

		virtual QObject* GetQObject () = 0;
	protected:
		virtual void sslErrors (const QList<QSslError>&, const ISslErrorsReaction_ptr&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ICanHaveSslErrors,
		"org.LeechCraft.Azoth.ICanHaveSslErrors/1.0")
