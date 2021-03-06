/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "readitlaterservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QtDebug>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include "readitlaterauthwidget.h"
#include "readitlaterapi.h"
#include "readitlateraccount.h"

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	ReadItLaterService::ReadItLaterService (ICoreProxy_ptr proxy)
	: CoreProxy_ (std::move (proxy))
	, ReadItLaterApi_ (new ReadItLaterApi)
	{
	}

	void ReadItLaterService::Prepare ()
	{
		RestoreAccounts ();
	}

	IBookmarksService::Features ReadItLaterService::GetFeatures () const
	{
		return FCanRegisterAccount;
	}

	QObject* ReadItLaterService::GetQObject ()
	{
		return this;
	}

	QString ReadItLaterService::GetServiceName () const
	{
		return "Read It Later";
	}

	QIcon ReadItLaterService::GetServiceIcon () const
	{
		return QIcon ("lcicons:/poshuku/onlinebookmarks:readitlater/resources/images/readitlater.ico");
	}

	QWidget* ReadItLaterService::GetAuthWidget ()
	{
		return new ReadItLaterAuthWidget ();
	}

	void ReadItLaterService::CheckAuthData (const QVariantMap& map)
	{
		const QString login = map ["Login"].toString ();
		const QString password = map ["Password"].toString ();
		if (login.isEmpty () || password.isEmpty ())
			return;

		Request req;
		req.Type_ = OTAuth;
		req.Login_ = login;
		req.Password_ = password;

		SendRequest (ReadItLaterApi_->GetAuthUrl (),
				ReadItLaterApi_->GetAuthPayload (login, password),
				req);
	}

	void ReadItLaterService::RegisterAccount(const QVariantMap& map)
	{
		const QString login = map ["Login"].toString ();
		const QString password = map ["Password"].toString ();
		if (login.isEmpty () || password.isEmpty ())
			return;

		Request req;
		req.Type_ = OTRegister;
		req.Login_ = login;
		req.Password_ = password;

		SendRequest (ReadItLaterApi_->GetRegisterUrl (),
				ReadItLaterApi_->GetRegisterPayload (login, password),
				req);
	}

	void ReadItLaterService::UploadBookmarks (QObject *accObj, const QVariantList& bookmarks)
	{
		IAccount *account = qobject_cast<IAccount*> (accObj);
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< "isn't an IAccount object"
					<< accObj;
			return;
		}

		QByteArray uploadBookmarks = ReadItLaterApi_->GetUploadPayload (account->GetLogin(),
				account->GetPassword (), bookmarks);

		if (uploadBookmarks.isEmpty ())
			return;

		Request req;
		req.Type_ = OTUpload;
		req.Login_ = account->GetLogin ();
		req.Password_ = account->GetPassword ();

		SendRequest (ReadItLaterApi_->GetUploadUrl (),
				uploadBookmarks,
				req);
	}

	void ReadItLaterService::DownloadBookmarks (QObject *accObj, const QDateTime& from)
	{
		IAccount *account = qobject_cast<IAccount*> (accObj);
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< "isn't an IAccount object"
					<< accObj;
			return;
		}

		QByteArray downloadBookmarks = ReadItLaterApi_->GetDownloadPayload (account->GetLogin(),
				account->GetPassword (), from);

		Request req;
		req.Type_ = OTDownload;
		req.Login_ = account->GetLogin ();
		req.Password_ = account->GetPassword ();
		Account2ReplyContent_ [account].clear ();

		SendRequest (ReadItLaterApi_->GetDownloadUrl (),
				downloadBookmarks,
				req);
	}

	ReadItLaterAccount* ReadItLaterService::GetAccountByName (const QString& login)
	{
		for (const auto account : Accounts_)
			if (account->GetLogin () == login)
				return account;

		return nullptr;
	}

	void ReadItLaterService::SendRequest (const QString& urlSting,
			const QByteArray& payload, Request req)
	{
		QUrl url = QUrl::fromEncoded (urlSting.toUtf8 () + payload);
		QNetworkRequest request (url);
		QNetworkReply *reply = CoreProxy_->
				GetNetworkAccessManager ()->get (request);

		Reply2Request_ [reply] = req;

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (getReplyFinished ()));

		connect (reply,
				SIGNAL (readyRead ()),
				this,
				SLOT (readyReadReply ()));
	}

	void ReadItLaterService::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Poshuku_OnlineBookmarks_ReadItLater_Accounts");

		QObjectList list;
		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QByteArray data = settings
					.value ("SerializedData").toByteArray ();

			ReadItLaterAccount *acc = ReadItLaterAccount::Deserialize (data, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unserializable acount"
						<< i;
				continue;
			}
			Accounts_ << acc;
			list << acc->GetQObject ();
		}

		emit accountAdded (list);
	}

	void ReadItLaterService::getReplyFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "isn't a QNetworkReply";
			return;
		}

		if (Reply2Request_ [reply].Type_ == OTDownload)
		{
			if (const auto account = GetAccountByName (Reply2Request_ [reply].Login_))
			{
				const auto& downloadedBookmarks = ReadItLaterApi_->GetDownloadedBookmarks (Account2ReplyContent_ [account]);
				if (!downloadedBookmarks.isEmpty ())
				{
					account->AppendDownloadedBookmarks (downloadedBookmarks);
					account->SetLastDownloadDateTime (QDateTime::currentDateTime ());
					emit gotBookmarks (account, downloadedBookmarks);
				}
			}
		}

		reply->deleteLater ();
	}

	void ReadItLaterService::readyReadReply ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "isn't a QNetworkReply";
			return;
		}

		const QVariant& result = reply->attribute (QNetworkRequest::HttpStatusCodeAttribute);
		QString msg;
		Priority priority = Priority::Info;
		switch (result.toInt ())
		{
		case 200:
			if (Reply2Request_ [reply].Type_ == OTAuth ||
					Reply2Request_ [reply].Type_ == OTRegister)
			{
				auto account = new ReadItLaterAccount (Reply2Request_ [reply].Login_, this);
				account->SetPassword (Reply2Request_ [reply].Password_);
				Accounts_ << account;
				saveAccounts ();
				emit accountAdded (QObjectList () << account->GetQObject ());
				switch (Reply2Request_ [reply].Type_)
				{
				case OTAuth:
					priority = Priority::Info;
					msg = tr ("Authentication has finished successfully.");
					break;
				case OTRegister:
					priority = Priority::Info;
					msg = tr ("Registration has finished successfully.");
					break;
				case OTDownload:
					break;
				case OTUpload:
					break;
				}
			}
			else
				switch (Reply2Request_ [reply].Type_)
				{
				case OTAuth:
					break;
				case OTRegister:
					break;
				case OTDownload:
					Account2ReplyContent_ [GetAccountByName (Reply2Request_ [reply].Login_)]
							.append (reply->readAll ());
					break;
				case OTUpload:
					ReadItLaterAccount *account = GetAccountByName (Reply2Request_ [reply].Login_);
					if (account)
						account->SetLastUploadDateTime (QDateTime::currentDateTime ());
					emit bookmarksUploaded ();
					break;
				}
			break;
		case 400:
			qWarning () << Q_FUNC_INFO
					<< "X-Error contents:"
					<< reply->rawHeader ("X-Error");
			msg = tr ("Invalid request. Please report to developers.");
			priority = Priority::Warning;
			break;
		case 401:
			msg = tr ("Incorrect username or password.");
			priority = Priority::Warning;
			break;
		case 403:
			msg = tr ("Rate limit exceeded, please wait a little bit before resubmitting.");
			priority = Priority::Warning;
			break;
		case 503:
			msg = tr ("Read It Later's sync server is down for scheduled maintenance.");
			priority = Priority::Warning;
			break;
		}
		auto e = Util::MakeNotification ("OnlineBookmarks",
				msg,
				priority);
		CoreProxy_->GetEntityManager ()->HandleEntity (e);
	}

	void ReadItLaterService::saveAccounts () const
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Poshuku_OnlineBookmarks_ReadItLater_Accounts");

		settings.beginWriteArray ("Accounts");

		for (int i = 0, size = Accounts_.size (); i < size; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData",
					Accounts_.at (i)->Serialize ());
		}

		settings.endArray ();
		settings.sync ();
	}

	void ReadItLaterService::removeAccount (QObject* accObj)
	{
		auto account = qobject_cast<ReadItLaterAccount*> (accObj);
		if (Accounts_.removeAll (account))
		{
			accObj->deleteLater ();
			saveAccounts ();
		}
	}

}
}
}
}
