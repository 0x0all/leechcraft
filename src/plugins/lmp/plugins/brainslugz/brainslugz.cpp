/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "brainslugz.h"
#include <util/util.h>
#include "checktab.h"
#include "progressmodelmanager.h"

namespace LC
{
namespace LMP
{
namespace BrainSlugz
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("lmp_brainslugz");

		ProgressModelManager_ = new ProgressModelManager { this };

		CoreProxy_ = proxy;

		CheckTC_ = TabClassInfo
		{
			GetUniqueID () + ".CheckTab",
			GetName (),
			GetInfo (),
			GetIcon (),
			0,
			TFOpenableByRequest | TFSingle
		};
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.BrainSlugz";
	}

	QString Plugin::GetName () const
	{
		return "LMP BrainSlugz";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Check if your collection misses some albums or EPs!");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { CheckTC_ };
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.General";
		return result;
	}

	QAbstractItemModel* Plugin::GetRepresentation () const
	{
		return ProgressModelManager_->GetModel ();
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LmpProxy_ = proxy;
	}

	void Plugin::TabOpenRequested (const QByteArray& tc)
	{
		if (tc == CheckTC_.TabClass_)
		{
			if (!OpenedTab_)
			{
				OpenedTab_ = new CheckTab { LmpProxy_, CoreProxy_, CheckTC_, this };

				connect (OpenedTab_,
						SIGNAL (removeTab (QWidget*)),
						this,
						SIGNAL (removeTab (QWidget*)));
				connect (OpenedTab_,
						SIGNAL (checkStarted (Checker*)),
						ProgressModelManager_,
						SLOT (handleCheckStarted (Checker*)));
			}
			emit addNewTab ("BrainSlugz", OpenedTab_);
			emit raiseTab (OpenedTab_);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tc;
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_brainslugz, LC::LMP::BrainSlugz::Plugin)
