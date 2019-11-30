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

#include "xdg.h"
#include <QIcon>
#include <QFile>

namespace LC
{
namespace Util
{
namespace XDG
{
	QIcon GetAppIcon (const QString& name)
	{
		return GetAppPixmap (name);
	}

	QPixmap GetAppPixmap (const QString& name)
	{
		const auto prefixes =
		{
			"/usr/share/pixmaps/",
			"/usr/local/share/pixmaps/"
		};

		const auto sizes = { "192", "128", "96", "72", "64", "48", "36", "32" };
		const QStringList themes
		{
			"/usr/local/share/icons/hicolor/",
			"/usr/share/icons/hicolor/"
		};

		for (auto ext : { ".png", ".svg", ".xpm", ".jpg", "" })
		{
			for (auto prefix : prefixes)
				if (QFile::exists (prefix + name + ext))
					return { prefix + name + ext };

			for (auto themeDir : themes)
				for (const auto& size : sizes)
				{
					const auto& str = themeDir + size + 'x' + size + "/apps/" + name + ext;
					if (QFile::exists (str))
						return { str };
				}
		}

		return {};
	}
}
}
}
