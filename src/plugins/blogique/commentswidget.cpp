/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin <MaledicutsDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "commentswidget.h"
#include <QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QMessageBox>
#include <interfaces/core/ientitymanager.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/tooltipitem.h>
#include <util/qml/themeimageprovider.h>
#include <util/sll/containerconversions.h>
#include <util/sys/paths.h>
#include <util/xpc/util.h>
#include "commentsmanager.h"
#include "commentsmodel.h"
#include "core.h"
#include "sortcommentsproxymodel.h"
#include "xmlsettingsmanager.h"
#include "addcommentdialog.h"

namespace LC
{
namespace Blogique
{
	CommentsWidget::CommentsWidget (QWidget *parent)
	: QWidget (parent)
	, View_ (new QQuickWidget)
	, CommentsModel_ (new CommentsModel (this))
	, ProxyModel_ (new SortCommentsProxyModel (this, this))
	{
		Ui_.setupUi (this);
		layout ()->addWidget (View_);

		ProxyModel_->setSourceModel (CommentsModel_);

		View_->setResizeMode (QQuickWidget::SizeRootObjectToView);
		View_->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);

		auto context = View_->rootContext ();
		context->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (Core::Instance ()
						.GetCoreProxy ()->GetColorThemeManager (), this));
		context->setContextProperty ("commentsModel", ProxyModel_);
		context->setContextProperty ("parentWidget", this);

		auto engine = View_->engine ();
		engine->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (Core::Instance ().GetCoreProxy ()));
		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			engine->addImportPath (cand);

		View_->setSource (QUrl::fromLocalFile (Util::GetSysPath (Util::SysPath::QML,
				"blogique", "commentsview.qml")));

		connect (View_->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLinkActivated (QString)));
		connect (View_->rootObject (),
				SIGNAL (deleteComment (QString, int)),
				this,
				SLOT (handleDeleteComment (QString, int)));
		connect (View_->rootObject (),
				SIGNAL (markCommentAsRead (QString, int)),
				this,
				SLOT (handleMarkCommentAsRead (QString, int)));
		connect (View_->rootObject (),
				SIGNAL (addComment (QString, int, int)),
				this,
				SLOT (handleAddComment (QString, int, int)));

		ReadComments_ = Util::AsSet (XmlSettingsManager::Instance ().property ("ReadComments").value<CommentIDs_t> ());

		ProxyModel_->sort (0, Qt::DescendingOrder);
		FillModel ();

		connect (Core::Instance ().GetCommentsManager (),
				SIGNAL (commentsUpdated ()),
				this,
				SLOT (handleCommentsUpdated ()));
	}

	QString CommentsWidget::GetName () const
	{
		return tr ("Comments");
	}

	CommentEntry CommentsWidget::GetRecentCommentFromIndex (const QModelIndex& index) const
	{
		return Item2RecentComment_.value (CommentsModel_->itemFromIndex (index));
	}

	void CommentsWidget::FillModel ()
	{
		AddItemsToModel (Core::Instance ().GetCommentsManager ()->GetComments ());
	}

	void CommentsWidget::AddItemsToModel (const QList<CommentEntry>& comments)
	{
		for (const auto& comment : comments)
		{
			if (RecentComments_.contains (comment))
				continue;

			CommentID cid;
			cid.AccountID_ = comment.AccountID_;
			cid.CommentID_ = comment.CommentID_;

			if (ReadComments_.contains (cid))
				continue;

			QStandardItem *item = new QStandardItem;
			item->setData (comment.AccountID_, CommentsModel::AccountID);
			item->setData (comment.EntrySubject_, CommentsModel::EntrySubject);
			item->setData (comment.EntryUrl_, CommentsModel::EntryUrl);
			item->setData (comment.EntryID_, CommentsModel::EntryID);
			item->setData (comment.CommentSubject_, CommentsModel::CommentSubject);
			item->setData (comment.CommentText_, CommentsModel::CommentBody);
			item->setData (comment.CommentAuthor_, CommentsModel::CommentAuthor);
			item->setData (QLocale {}.toString (comment.CommentDateTime_, QLocale::ShortFormat),
						CommentsModel::CommentDate);
			item->setData (comment.CommentUrl_, CommentsModel::CommentUrl);
			item->setData (comment.CommentID_, CommentsModel::CommentID);

			Item2RecentComment_ [item] = comment;
			RecentComments_ << comment;

			CommentsModel_->appendRow (item);
		}
	}

	CommentEntry CommentsWidget::GetComment (const QString& accountId, int commentId) const
	{
		for (const auto& comment : RecentComments_)
			if (comment.CommentID_ == commentId &&
					comment.AccountID_ == accountId.toUtf8 ())
				return comment;
		return CommentEntry ();
	}

	void CommentsWidget::handleLinkActivated (const QString& url)
	{
		Core::Instance ().GetCoreProxy ()->GetEntityManager ()->
				HandleEntity (Util::MakeEntity (url,
						QString (),
						OnlyHandle | FromUserInitiated));
	}

	void CommentsWidget::handleDeleteComment (const QString& accountId, int commentId)
	{
		const auto& comment = GetComment (accountId, commentId);
		if (!comment.isValid ())
			return;

		if (auto account = Core::Instance ().GetAccountByID (comment.AccountID_))
		{
			auto res = QMessageBox::question (this, "LeechCraft",
					tr ("Do you want to delete whole comment thread too?"),
					QMessageBox::Yes | QMessageBox::No);
			bool deleteThread = res == QMessageBox::Yes;

			account->DeleteComment (commentId, deleteThread);
		}
	}

	void CommentsWidget::handleMarkCommentAsRead (const QString& accountId, int commentId)
	{
		auto comment = GetComment (accountId, commentId);
		if (!comment.isValid ())
			return;

		CommentID cid;
		cid.AccountID_ = accountId.toUtf8 ();
		cid.CommentID_ = commentId;
		ReadComments_.insert (cid);

		XmlSettingsManager::Instance ().setProperty ("ReadComments",
				QVariant::fromValue<CommentIDs_t> (ReadComments_.values ()));

		CommentEntry ce;
		ce.AccountID_ = comment.AccountID_;
		ce.CommentID_ = comment.CommentID_;
		if (auto item = Item2RecentComment_.key (ce))
			CommentsModel_->removeRow (item->index ().row ());
	}

	void CommentsWidget::handleAddComment (const QString& accountId, int entryID, int commentId)
	{
		if (auto account = Core::Instance ().GetAccountByID (accountId.toUtf8 ()))
		{
			AddCommentDialog dlg;
			if (dlg.exec () == QDialog::Rejected)
				return;

			CommentEntry ce;
			ce.AccountID_ = accountId.toUtf8 ();
			ce.CommentSubject_ = dlg.GetSubject ();
			ce.CommentText_ = dlg.GetText ();
			ce.ParentCommentID_ = commentId;
			ce.EntryID_ = entryID;
			account->AddComment (ce);
		}
	}

	void CommentsWidget::handleCommentsUpdated ()
	{
		CommentsModel_->clear ();
		Item2RecentComment_.clear ();
		RecentComments_.clear ();
		FillModel ();
	}

	QDataStream& operator<< (QDataStream& out, const LC::Blogique::CommentsWidget::CommentID& comment)
	{
		out << static_cast<qint8> (1)
				<< comment.AccountID_
				<< comment.CommentID_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, LC::Blogique::CommentsWidget::CommentID& comment)
	{
		qint8 version = 0;
		in >> version;
		if (version > 0)
			in >> comment.AccountID_
					>> comment.CommentID_;
		return in;
	}

	uint qHash (const LC::Blogique::CommentsWidget::CommentID& cid)
	{
		return qHash (cid.AccountID_) + ::qHash (cid.CommentID_);
	}

}
}
