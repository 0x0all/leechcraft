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
#include <QModelIndex>
#include <interfaces/iinfo.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/an/ianemitter.h>

namespace LeechCraft
{
namespace Azoth
{
	class ServiceDiscoveryWidget;
	class MainWidget;
	class ConsoleWidget;
	class MicroblogsTab;
	class ServerHistoryWidget;

	class Plugin : public QObject
				 , public IInfo
				 , public IPluginReady
				 , public IHaveTabs
				 , public IHaveRecoverableTabs
				 , public IHaveSettings
				 , public IJobHolder
				 , public IActionsExporter
				 , public IEntityHandler
				 , public IHaveShortcuts
				 , public IANEmitter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPluginReady
				IHaveTabs
				IHaveRecoverableTabs
				IHaveSettings
				IJobHolder
				IActionsExporter
				IEntityHandler
				IHaveShortcuts
				IANEmitter)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth")

		MainWidget *MW_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		TabClasses_t TabClasses_;
		TabClassInfo ServerHistoryTC_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;
		QStringList Provides () const override;

		QSet<QByteArray> GetExpectedPluginClasses () const override;
		void AddPlugin (QObject*) override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		QAbstractItemModel* GetRepresentation () const override;
		IJobHolderRepresentationHandler_ptr CreateRepresentationHandler () override;

		QList<QAction*> GetActions (ActionsEmbedPlace) const override;
		QMap<QString, QList<QAction*>> GetMenuActions () const override;

		EntityTestHandleResult CouldHandle (const Entity&) const override;
		void Handle (Entity) override;

		TabClasses_t GetTabClasses () const override;
		void TabOpenRequested (const QByteArray&) override;

		void RecoverTabs (const QList<TabRecoverInfo>&) override;
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const override;

		void SetShortcut (const QString&, const QKeySequences_t&) override;
		QMap<QString, ActionInfo> GetActionInfo() const override;

		QList<ANFieldData> GetANFields () const override;
	private :
		void InitShortcuts ();
		void InitAccActsMgr ();
		void InitSettings ();
		void InitMW ();
		void InitSignals ();
		void InitTabClasses ();
	public slots:
		void handleSDWidget (ServiceDiscoveryWidget*);
		void handleMicroblogsTab (MicroblogsTab*);
		void handleServerHistoryTab (ServerHistoryWidget*);
	private slots:
		void handleMWLocation (Qt::DockWidgetArea);
		void handleMWFloating (bool);
		void handleMoreThisStuff (const QString&);
		void handleConsoleWidget (ConsoleWidget*);
	signals:
		void gotEntity (const LeechCraft::Entity&) override;

		void addNewTab (const QString&, QWidget*) override;
		void removeTab (QWidget*) override;
		void changeTabName (QWidget*, const QString&) override;
		void changeTabIcon (QWidget*, const QIcon&) override;
		void statusBarChanged (QWidget*, const QString&) override;
		void raiseTab (QWidget*) override;

		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace) override;
	};
}
}
