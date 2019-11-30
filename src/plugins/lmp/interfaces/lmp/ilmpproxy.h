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
#include <QFileInfo>
#include <QMap>
#include <QVariant>

class QPixmap;

namespace LC
{
namespace LMP
{
	class ILocalCollection;
	class ITagResolver;
	class ILMPUtilProxy;
	class ILMPGuiProxy;

	class ILMPProxy
	{
	public:
		virtual ~ILMPProxy () {}

		virtual ILocalCollection* GetLocalCollection () const = 0;

		virtual ITagResolver* GetTagResolver () const = 0;

		virtual const ILMPUtilProxy* GetUtilProxy () const = 0;

		virtual const ILMPGuiProxy* GetGuiProxy () const = 0;

		virtual void PreviewRelease (const QString& artist, const QString& release,
				const QList<QPair<QString, int>>& tracks) const = 0;
	};

	using ILMPProxy_ptr = ILMPProxy*;
}
}

Q_DECLARE_INTERFACE (LC::LMP::ILMPProxy, "org.LeechCraft.LMP.ILMPProxy/1.0")
