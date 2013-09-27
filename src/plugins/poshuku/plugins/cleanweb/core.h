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

#pragma once

#include <QAbstractItemModel>
#include <QHash>
#include <QStringList>
#include <QNetworkReply>
#include <QDateTime>
#include <QWebPage>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/poshuku/poshukutypes.h>
#include <interfaces/core/ihookproxy.h>
#include "filter.h"

class QNetworkRequest;
class QWebPage;
class QWebView;
class QWebHitTestResult;

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
	class FlashOnClickPlugin;
	class FlashOnClickWhitelist;
	class UserFiltersModel;


	struct HidingWorkerResult
	{
		QPointer<QWebFrame> Frame_;
		QStringList Selectors_;
	};

	class Core : public QAbstractItemModel
	{
		Q_OBJECT

		FlashOnClickPlugin *FlashOnClickPlugin_;
		FlashOnClickWhitelist *FlashOnClickWhitelist_;
		UserFiltersModel *UserFilters_;

		QList<Filter> Filters_;
		QObjectList Downloaders_;
		QStringList HeaderLabels_;

		struct PendingJob
		{
			QString FullName_;
			QString FileName_;
			QString Subscr_;
			QUrl URL_;
		};
		QMap<int, PendingJob> PendingJobs_;

		QHash<QWebFrame*, QList<QUrl>> MoreDelayedURLs_;

		ICoreProxy_ptr Proxy_;

		Core ();
	public:
		static Core& Instance ();
		void Release ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		int columnCount (const QModelIndex& = QModelIndex ()) const;
		QVariant data (const QModelIndex&, int) const;
		QVariant headerData (int, Qt::Orientation, int) const;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex& = QModelIndex ()) const;

		bool CouldHandle (const Entity&) const;
		void Handle (Entity);
		QAbstractItemModel* GetModel ();
		void Remove (const QModelIndex&);

		void HandleInitialLayout (QWebPage*, QWebFrame*);
		QNetworkReply* Hook (LeechCraft::IHookProxy_ptr,
				QNetworkAccessManager*,
				QNetworkAccessManager::Operation*,
				QIODevice**);
		void HandleExtension (LeechCraft::IHookProxy_ptr,
				QWebPage*,
				QWebPage::Extension,
				const QWebPage::ExtensionOption*,
				QWebPage::ExtensionReturn*);
		void HandleContextMenu (const QWebHitTestResult&,
				QWebView*, QMenu*,
				WebViewCtxMenuStage);

		bool ShouldReject (const QNetworkRequest&, QString*) const;

		UserFiltersModel* GetUserFiltersModel () const;
		FlashOnClickPlugin* GetFlashOnClick ();
		FlashOnClickWhitelist* GetFlashOnClickWhitelist ();

		bool Exists (const QUrl& url) const;
		bool Exists (const QString& name) const;

		/** Parses the abp:-schemed url, gets subscription
		 * name and real url from there and adds it via Load().
		 *
		 * Returns true if the url is added successfully or
		 * false otherwise (if url is malformed or such
		 * subscription already exists, for example).
		 *
		 * @param[in] url The abp:-schemed URL.
		 *
		 * @return Whether addition was successful.
		 */
		bool Add (const QUrl& url);

		/** Loads the subscription from the url with the name
		 * subscrName. Returns true if the load delegation was
		 * successful, otherwise returns false.
		 *
		 * url is expected to be a "real" URL of the filters
		 * file — with, say, http:// scheme.
		 *
		 * Returns true if the url is added successfully or
		 * false otherwise (if url is malformed or such
		 * subscription already exists, for example).
		 *
		 * @param[in] url Real URL of the file with the filters.
		 * @param[in] subscrName The name if this subscription.
		 *
		 * @return Whether addition was successful.
		 */
		bool Load (const QUrl& url, const QString& subscrName);
	private:
		bool Matches (const FilterItem&, const QString&, const QByteArray&, const QString&) const;
		void HandleProvider (QObject*);

		void AddFilter (const Filter&);
		void Parse (const QString&);

		/** Removes the subscription at
		 * ~/.leechcraft/cleanweb/filename.
		 */
		void Remove (const QString& filename);
		void WriteSettings ();
		void ReadSettings ();
		bool AssignSD (const SubscriptionData&);
	private slots:
		void handleParsed ();
		void update ();
		void handleJobFinished (int);
		void handleJobError (int, IDownload::Error);
		void handleFrameLayout (QPointer<QWebFrame>);
		void hidingElementsFound ();
		void hideElementsChunk (HidingWorkerResult);
		void delayedRemoveElements (QPointer<QWebFrame>, const QUrl&);
		void moreDelayedRemoveElements ();
		void handleFrameDestroyed ();
	signals:
		void delegateEntity (const LeechCraft::Entity&,
				int*, QObject**);
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}
