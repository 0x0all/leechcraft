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

#pragma once

#include <QAbstractItemModel>
#include <QMap>
#include <interfaces/structures.h>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/isyncable.h>
#include "description.h"
#include "searchhandler.h"

class IWebBrowser;

namespace LC::SeekThru
{
	class Core : public QAbstractItemModel
	{
		Q_OBJECT

		QMap<QString, QObject*> Providers_;
		QList<Description> Descriptions_;
		QStringList Headers_;
		ICoreProxy_ptr Proxy_;

		static const QString OS_;

		Core ();
	public:
		enum Roles
		{
			RoleDescription = Qt::UserRole + 1,
			RoleContact,
			RoleTags,
			RoleLongName,
			RoleDeveloper,
			RoleAttribution,
			RoleRight,
			RoleAdult,
			RoleLanguages
		};

		static Core& Instance ();

		void DoDelayedInit ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		int columnCount (const QModelIndex& = QModelIndex ()) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const override;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = QModelIndex ()) const override;

		void SetProvider (QObject*, const QString&);
		bool CouldHandle (const Entity&) const;
		void Handle (const Entity&);

		/** Fetches the searcher from the url.
			*
			* @param[in] url The url with the search description.
			*/
		void Add (const QUrl& url);
		void Remove (const QModelIndex&);
		void SetTags (const QModelIndex&, const QStringList&);
		QStringList GetCategories () const;
		IFindProxy_ptr GetProxy (const LC::Request&);
		IWebBrowser* GetWebBrowser () const;
		void HandleEntity (const QString&, const QString& = QString ());
	private:
		void SetTags (int, const QStringList&);
		QStringList ComputeUniqueCategories () const;
		QList<Description> FindMatchingHRTag (const QString&) const;
		Description ParseData (const QString&, const QString&);
		void ReadSettings ();
		void WriteSettings ();
	public:
		bool HandleDADescrAdded (QDataStream&);
		bool HandleDADescrRemoved (QDataStream&);
		bool HandleDATagsChanged (QDataStream&);
	signals:
		void error (const QString&);
		void categoriesChanged (const QStringList&, const QStringList&);
	};
}
