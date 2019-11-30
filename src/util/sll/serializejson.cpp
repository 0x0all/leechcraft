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

#include "serializejson.h"
#include <QJsonDocument>
#include <QFile>
#include "either.h"

namespace LC
{
namespace Util
{
	QByteArray SerializeJson (const QVariant& var, bool compact)
	{
		return QJsonDocument::fromVariant (var)
				.toJson (compact ? QJsonDocument::Compact : QJsonDocument::Indented);
	}

	using SerializeResult_t = Either<QString, Void>;

	UTIL_SLL_API SerializeResult_t SerializeJsonToFile (const QString& filename, const QVariant& var, bool compact)
	{
		QFile file { filename };
		if (!file.open (QIODevice::WriteOnly))
			return SerializeResult_t::Left (file.errorString ());

		if (!file.write (SerializeJson (var, compact)))
			return SerializeResult_t::Left (file.errorString ());

		return SerializeResult_t::Right ({});
	}
}
}
