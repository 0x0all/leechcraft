/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
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

#include "colorschemesmanager.h"
#include <optional>
#include <QDir>
#include <QSettings>
#include <QSet>
#include <QStandardPaths>
#include <qtermwidget.h>
#include <util/sll/prelude.h>

namespace LC
{
namespace Eleeminator
{
	ColorSchemesManager::ColorSchemesManager (QObject* parent)
	: QObject { parent }
	{
		LoadKonsoleSchemes ();

		Schemes_ += Util::Map (QTermWidget::availableColorSchemes (),
				[] (const QString& name) { return Scheme { name, name }; });

		FilterDuplicates ();

		std::sort (Schemes_.begin (), Schemes_.end (), Util::ComparingBy (&Scheme::Name_));
	}

	QList<ColorSchemesManager::Scheme> ColorSchemesManager::GetSchemes () const
	{
		return Schemes_;
	}

	namespace
	{
		QStringList CollectSchemes (const QString& dir)
		{
			return Util::Map (QDir { dir }.entryList ({ "*.colorscheme" }),
					[&dir] (QString str) { return str.prepend (dir); });
		}

		using MaybeScheme_t = std::optional<ColorSchemesManager::Scheme>;

		MaybeScheme_t ParseScheme (const QString& filename)
		{
			QSettings settings { filename, QSettings::IniFormat };
			settings.setIniCodec ("UTF-8");
			auto name = settings.value ("Description").toString ();
			if (name.isEmpty ())
				name = QFileInfo { filename }.baseName ();

			return { { name, filename } };
		}
	}

	void ColorSchemesManager::LoadKonsoleSchemes ()
	{
		const auto& pathCandidates = Util::Map (QStandardPaths::standardLocations (QStandardPaths::GenericDataLocation),
				[] (const QString& str) { return str + "/konsole/"; });

		const auto& filenames = Util::ConcatMap (pathCandidates, &CollectSchemes);
		Schemes_ += Util::Map (Util::Filter (Util::Map (filenames, &ParseScheme),
						[] (const MaybeScheme_t& scheme) { return static_cast<bool> (scheme); }),
					[] (const MaybeScheme_t& scheme) { return *scheme; });
	}

	void ColorSchemesManager::FilterDuplicates ()
	{
		QSet<QString> names;

		for (auto i = Schemes_.begin (); i != Schemes_.end (); )
		{
			const auto normalized = QString { i->Name_ }.remove (' ').toLower ();

			if (names.contains (normalized))
				i = Schemes_.erase (i);
			else
			{
				names << normalized;
				++i;
			}
		}
	}
}
}
