/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
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

#include <QDateTime>
#include <QStringList>
#include <QUrl>

namespace LC
{
namespace Blasq
{
namespace Vangog
{
	enum class Access
	{
		Private,
		Public
	};

	struct Author
	{
		QString Name_;
		QUrl Image_;
	};

	struct Thumbnail
	{
		QUrl Url_;
		int Width_;
		int Height_;

		Thumbnail ()
		: Width_ (0)
		, Height_ (0)
		{}
	};

	struct Album
	{
		QByteArray ID_;
		QString Title_;
		QString Description_;
		QDateTime Published_;
		QDateTime Updated_;
		Access Access_;
		Author Author_;
		int NumberOfPhoto_;
		quint64 BytesUsed_;
		QList<Thumbnail> Thumbnails_;

		Album ()
		: Access_ (Access::Private)
		, NumberOfPhoto_ (0)
		, BytesUsed_ (0)
		{}
	};

	struct Exif
	{
		QString Manufacturer_;
		QString Model_;
		int FNumber_;
		float Exposure_;
		bool Flash_;
		float FocalLength_;
		int ISO_;

		Exif ()
		: FNumber_ (0)
		, Exposure_ (0.0)
		, Flash_ (false)
		, FocalLength_ (0.0)
		, ISO_ (0)
		{}
	};

	struct Photo
	{
		QByteArray ID_;
		QString Title_;
		QDateTime Published_;
		QDateTime Updated_;
		Access Access_;
		QByteArray AlbumID_;
		int Width_;
		int Height_;
		quint64 Size_;
		Exif Exif_;
		QUrl Url_;
		QStringList Tags_;
		QList<Thumbnail> Thumbnails_;

		Photo ()
		: Access_ (Access::Private)
		, Width_ (0)
		, Height_ (0)
		, Size_ (0)
		{}
	};
}
}
}
