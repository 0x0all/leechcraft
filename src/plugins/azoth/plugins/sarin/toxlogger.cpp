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

#include "toxlogger.h"
#include <QFile>
#include <QDir>
#include <QtDebug>
#include <util/sys/paths.h>

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	ToxLogger::ToxLogger (const QString& name)
	: Name_ { name }
	{
	}

	namespace
	{
		QByteArray LogLevelToMarker (TOX_LOG_LEVEL level)
		{
			switch (level)
			{
			case TOX_LOG_LEVEL_TRACE:
				return "[TRACE] ";
			case TOX_LOG_LEVEL_DEBUG:
				return "[DBG]   ";
			case TOX_LOG_LEVEL_INFO:
				return "[INFO]  ";
			case TOX_LOG_LEVEL_WARNING:
				return "[WARN]  ";
			case TOX_LOG_LEVEL_ERROR:
				return "[ERR]   ";
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown debug level"
					<< level;
			return "[UN]";
		}
	}

	void ToxLogger::Log (TOX_LOG_LEVEL level,
			const char *srcFile, uint32_t line, const char *func,
			const char *message)
	{
		const auto& path = Util::CreateIfNotExists ("azoth/sarin/logs").filePath (Name_ + ".log");
		QFile file { path };
		if (!file.open (QIODevice::WriteOnly | QIODevice::Append))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open file"
					<< path
					<< ":"
					<< file.errorString ();
			return;
		}

		file.write (QString { "%1 %2 %4:%3: `%5`" }
				.arg (QString::fromLatin1 (LogLevelToMarker (level)))
				.arg (QString::fromUtf8 (srcFile))
				.arg (line)
				.arg (QString::fromUtf8 (func))
				.arg (QString::fromUtf8 (message))
				.toUtf8 ());
	}
}
}
}
