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

#include "potorchu.h"
#include <QIcon>
#include <util/util.h>
#include "visualfilter.h"

namespace LC
{
namespace LMP
{
namespace Potorchu
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("lmp_potorchu");
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.Potorchu";
	}

	QString Plugin::GetName () const
	{
		return "LMP Potorchu";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Visualization effects for LMP.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.FiltersProvider";
		return result;
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LmpProxy_ = proxy;
	}

	QList<EffectInfo> Plugin::GetEffects () const
	{
		return
		{
			{
				GetUniqueID () + ".Filter",
				tr ("Visual effects"),
				{},
				false,
				[this] (const QByteArray&, IPath*) -> IFilterElement*
				{
					return new VisualFilter
						{
							GetUniqueID () + ".Filter",
							LmpProxy_
						};
				}
			}
		};
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_potorchu, LC::LMP::Potorchu::Plugin);
