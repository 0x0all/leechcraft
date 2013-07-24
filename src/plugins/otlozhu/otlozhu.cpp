/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "otlozhu.h"
#include <QIcon>
#include <util/util.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/itagsmanager.h>
#include "todotab.h"
#include "core.h"
#include "todomanager.h"
#include "todostorage.h"
#include "syncproxy.h"

namespace LeechCraft
{
namespace Otlozhu
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("otlozhu");
		Core::Instance ().SetProxy (proxy);

		connect (&Core::Instance (),
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));

		TCTodo_ = TabClassInfo
		{
			GetUniqueID () + "_todo",
			GetName (),
			GetInfo (),
			GetIcon (),
			20,
			TFOpenableByRequest | TFSingle | TFSuggestOpening
		};

		SyncProxy_ = new SyncProxy;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Otlozhu";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Otlozhu";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("A simple GTD-compatible ToDo manager.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/otlozhu/resources/images/otlozhu.svg");
		return icon;
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { TCTodo_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& id)
	{
		if (id == TCTodo_.TabClass_)
		{
			auto tab = new TodoTab (TCTodo_, this);
			emit addNewTab (TCTodo_.VisibleName_, tab);
			emit raiseTab (tab);

			connect (tab,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));

			connect (tab,
					SIGNAL (gotEntity (LeechCraft::Entity)),
					this,
					SIGNAL (gotEntity (LeechCraft::Entity)));
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown id"
					<< id;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		return e.Mime_ == "x-leechcraft/todo-item" ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity e)
	{
		auto mgr = Core::Instance ().GetTodoManager ();

		TodoItem_ptr item (new TodoItem ());
		item->SetTitle (e.Entity_.toString ());
		item->SetComment (e.Additional_ ["TodoBody"].toString ());

		const auto& tags = e.Additional_ ["Tags"].toStringList ();
		auto tm = Core::Instance ().GetProxy ()->GetTagsManager ();
		QStringList ids;
		std::transform (tags.begin (), tags.end (), std::back_inserter (ids),
				[tm] (const QString& tag) { return tm->GetID (tag); });
		item->SetTagIDs (ids);

		mgr->GetTodoStorage ()->AddItem (item);
	}

	ISyncProxy* Plugin::GetSyncProxy ()
	{
		return SyncProxy_;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_otlozhu, LeechCraft::Otlozhu::Plugin);
