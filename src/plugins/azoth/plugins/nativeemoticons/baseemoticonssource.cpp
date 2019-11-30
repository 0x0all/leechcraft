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

#include "baseemoticonssource.h"
#include <QtDebug>
#include <util/sys/resourceloader.h>
#include <util/sll/qtutil.h>

namespace LC
{
namespace Azoth
{
namespace NativeEmoticons
{
	BaseEmoticonsSource::BaseEmoticonsSource (const QString& suffix, QObject *parent)
	: QObject { parent }
	, EmoLoader_ { new Util::ResourceLoader { "azoth/emoticons/" + suffix, this } }
	{
		EmoLoader_->AddGlobalPrefix ();
		EmoLoader_->AddLocalPrefix ();
	}

	QAbstractItemModel* BaseEmoticonsSource::GetOptionsModel () const
	{
		return EmoLoader_->GetSubElemModel ();
	}

	QSet<QString> BaseEmoticonsSource::GetEmoticonStrings (const QString& pack) const
	{
		return ParseFile (pack).keys ().toSet ();
	}

	QHash<QImage, QString> BaseEmoticonsSource::GetReprImages (const QString& pack) const
	{
		QHash<QImage, QString> result;

		QSet<QString> knownPaths;
		for (const auto& pair : Util::Stlize (ParseFile (pack)))
		{
			const auto& path = pair.second;
			if (knownPaths.contains (path))
				continue;

			knownPaths << path;

			const auto& fullPath = EmoLoader_->GetIconPath (pack + "/" + path);
			const QImage img { fullPath };
			if (img.isNull ())
			{
				qWarning () << Q_FUNC_INFO
						<< path
						<< "in pack"
						<< pack
						<< "is null, got path:"
						<< fullPath;
				continue;
			}

			result [img] = pair.first;
		}

		return result;
	}

	QByteArray BaseEmoticonsSource::GetImage (const QString& pack, const QString& smile) const
	{
		const auto& hash = ParseFile (pack);
		if (!hash.contains (smile))
			return {};

		const auto& path = EmoLoader_->GetIconPath (pack + "/" + hash [smile]);
		QFile file { path };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< "for"
					<< pack
					<< smile
					<< file.errorString ();
			return {};
		}

		return file.readAll ();
	}
}
}
}
