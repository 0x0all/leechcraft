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

#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/idatafilter.h>
#include <interfaces/istartupwizard.h>
#include <interfaces/isyncable.h>
#include <interfaces/structures.h>

namespace LeechCraft
{
namespace SeekThru
{
	class SeekThru : public QObject
					, public IInfo
					, public IFinder
					, public IHaveSettings
					, public IEntityHandler
					, public IDataFilter
					, public IStartupWizard
					, public ISyncable
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IFinder
				IHaveSettings
				IEntityHandler
				IDataFilter
				IStartupWizard
				ISyncable)

		LC_PLUGIN_METADATA ("org.LeechCraft.SeekThru")

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QStringList Provides () const;
		QStringList Needs () const;
		QStringList Uses () const;
		void SetProvider (QObject*, const QString&);

		QStringList GetCategories () const;
		QList<IFindProxy_ptr> GetProxy (const LeechCraft::Request&);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		EntityTestHandleResult CouldHandle (const LeechCraft::Entity&) const;
		void Handle (LeechCraft::Entity);

		QString GetFilterVerb () const;
		QList<FilterVariant> GetFilterVariants (const QVariant&) const;

		QList<QWizardPage*> GetWizardPages () const;

		ISyncProxy* GetSyncProxy ();
	private slots:
		void handleError (const QString&);
	signals:
		void categoriesChanged (const QStringList&, const QStringList&);
	};
}
}
