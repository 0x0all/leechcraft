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

#include "fsmodel.h"
#include <QFileIconProvider>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "localcollection.h"

namespace LC
{
namespace LMP
{
	class FSIconProvider : public QFileIconProvider
	{
	public:
		QIcon icon (const QFileInfo& info) const
		{
			if (!info.isDir ())
				return QFileIconProvider::icon (info);

			const auto& path = info.absoluteFilePath ();
			const auto status = Core::Instance ().GetLocalCollection ()->GetDirStatus (path);
			if (status != LocalCollection::DirStatus::None)
				return Core::Instance ().GetProxy ()->
						GetIconThemeManager ()->GetIcon ("folder-bookmark");

			return QFileIconProvider::icon (info);
		}
	};

	FSModel::FSModel (QObject *parent)
	: DndActionsMixin<QFileSystemModel> (parent)
	{
		setIconProvider (new FSIconProvider);

		setSupportedDragActions (Qt::CopyAction);
	}
}
}
