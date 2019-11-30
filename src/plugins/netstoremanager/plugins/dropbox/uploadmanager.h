/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
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
#include <QPointer>
#include <QStandardItem>
#include <interfaces/netstoremanager/istorageaccount.h>

class QNetworkAccessManager;

namespace LC
{
namespace NetStoreManager
{
namespace DBox
{
	class Account;
	class DriveManager;

	class UploadManager : public QObject
	{
		Q_OBJECT

		Account *Account_;
		QString FilePath_;
		QNetworkAccessManager *NAM_;
		QStringList ParentId_;
		QStringList Id_;
	public:
		UploadManager (const QString& path, UploadType ut,
				const QByteArray& parentId, Account *account,
				const QByteArray& id = QByteArray ());
	private:
		void InitiateUploadSession ();
		void InitiateUpdateSession ();

	private slots:
		void handleUploadProgress (qint64 sent, qint64 total, const QString& filePath);
		void handleStatusChanged (const QString& status, const QString& filePath);
		void handleError (const QString& error, const QString& filePath);
		void handleFinished (const QString& id, const QString& filePath);
	signals:
		void uploadError (const QString& str, const QString& filePath);
		void uploadProgress (quint64 sent, quint64 total, const QString& filePath);
		void uploadStatusChanged (const QString& status, const QString& filePath);
		void finished (const QByteArray& id, const QString& filepath);
	};
}
}
}
