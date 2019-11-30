/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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
#include <QVariant>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "syncwidget.h"

typedef QList<LC::NetStoreManager::Change> Changes_t;
Q_DECLARE_METATYPE (Changes_t)

class QThread;

namespace LC
{
namespace NetStoreManager
{
	class AccountsManager;
	class FilesWatcherBase;
	class Syncer;

	typedef QHash<QByteArray, Change> Snapshot_t;

	class SyncManager : public QObject
	{
		Q_OBJECT

		AccountsManager *AM_;
		FilesWatcherBase *FilesWatcher_;
		QHash<QString, Syncer*> AccountID2Syncer_;
		QHash<Syncer*, QThread*> Syncer2Thread_;

	public:
		SyncManager (AccountsManager *am, QObject *parent = 0);

		void Release ();
	private:
		Syncer* CreateSyncer (IStorageAccount *isa, const QString& baseDir,
				const QString& remoteDir);
		void WriteSnapshots ();
		void ReadSnapshots ();
		Syncer* GetSyncerByID (const QByteArray& id) const;
		Syncer* GetSyncerByLocalPath (const QString& localPath) const;

	public slots:
		void handleDirectoriesToSyncUpdated (const QList<SyncerInfo>& map);

	private slots:
		void handleDirWasCreated (const QString& path);
		void handleDirWasRemoved (const QString& path);
		void handleFileWasCreated (const QString& path);
		void handleFileWasRemoved (const QString& path);
		void handleFileWasUpdated (const QString& path);
		void handleEntryWasMoved (const QString& oldPath, const QString& newPath);
		void handleEntryWasRenamed (const QString& oldName, const QString& newName);

		void handleGotListing (const QList<StorageItem>& items);
		void handleGotNewItem (const StorageItem& item, const QByteArray& parentId);
		void handleGotChanges (const QList<Change>& changes);
	};
}
}
