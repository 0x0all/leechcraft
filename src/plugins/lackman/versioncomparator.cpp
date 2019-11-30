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

#include "versioncomparator.h"
#include <QStringList>
#include <QtDebug>

namespace LC
{
namespace LackMan
{
	QString Numerize (QString version)
	{
		static const QStringList mods { "-rc", "-pre", "-beta", "-alpha" };
		static QStringList replacements;
		int modsSize = mods.size ();
		if (replacements.isEmpty ())
			for (int i = 0; i < modsSize; ++i)
				replacements << QString (".%1.").arg (-i - 1);

		for (int i = 0; i < modsSize; ++i)
			version.replace (mods.at (i), replacements.at (i));

		return version;
	}

	bool IsVersionLess (const QString& leftVer, const QString& rightVer)
	{
		if (leftVer == rightVer)
			return false;

		QString leftNum = Numerize (leftVer);
		QString rightNum = Numerize (rightVer);

#ifdef VERSIONCOMPARATOR_DEBUG
		qDebug () << leftVer << "->" << leftNum
				<< rightVer << "->" << rightNum;
#endif

		QStringList leftParts = leftNum.split ('.',
				QString::SkipEmptyParts);
		QStringList rightParts = rightNum.split ('.',
				QString::SkipEmptyParts);

		int maxSize = std::max (leftParts.size (), rightParts.size ());
		for (int i = leftParts.size (); i < maxSize; ++i)
			leftParts << "0";
		for (int i = rightParts.size (); i < maxSize; ++i)
			rightParts << "0";

#ifdef VERSIONCOMPARATOR_DEBUG
		qDebug () << leftParts << rightParts;
#endif

		for (int i = 0; i < maxSize; ++i)
		{
			int left = leftParts.at (i).toInt ();
			int right = rightParts.at (i).toInt ();
#ifdef VERSIONCOMPARATOR_DEBUG
			qDebug () << left << right;
#endif
			if (left < right)
				return true;
			else if (left > right)
				return false;
		}

		return false;
	}
}
}
