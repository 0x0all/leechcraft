/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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

	SerializeResult_t SerializeJsonToFile (const QString& filename, const QVariant& var, bool compact)
	{
		QFile file { filename };
		if (!file.open (QIODevice::WriteOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< "for writing:"
					<< file.errorString ();
			return SerializeResult_t::Left (file.errorString ());
		}

		if (!file.write (SerializeJson (var, compact)))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to write to file"
					<< file.fileName ()
					<< ":"
					<< file.errorString ();
			return SerializeResult_t::Left (file.errorString ());
		}

		return SerializeResult_t::Right ({});
	}
}
}
