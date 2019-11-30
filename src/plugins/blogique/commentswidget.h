/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin <MaledicutsDeMagog@gmail.com>
 *
 *  Boost Software License - Version 1.0 - August 17th, 2003
 *
 *  Permission is hereby granted, free of charge, to any person or organization
 *  obtaining a copy of the software and accompanying documentation covered by
 *  this license (the "Software") to use, reproduce, display, distribute,
 *  execute, and transmit the Software, and to prepare derivative works of the
 *  Software, and to permit third-parties to whom the Software is furnished to
 *  do so, all subject to the following:
 *
 *  The copyright notices in the Software and this entire statement, including
 *  the above license grant, this restriction and the following disclaimer,
 *  must be included in all copies of the Software, in whole or in part, and
 *  all derivative works of the Software, unless such copies or derivative
 *  works are solely in the form of machine-executable object code generated by
 *  a source language processor.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 *  SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 *  FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QSet>
#include "interfaces/blogique/iaccount.h"
#include "ui_commentswidget.h"

class QStandardItem;
class QQuickWidget;

namespace LC
{
namespace Blogique
{
	class CommentsModel;
	class SortCommentsProxyModel;

	class CommentsWidget : public QWidget
	{
		Q_OBJECT

		Ui::CommentsWidget Ui_;
		QQuickWidget * const View_;
		CommentsModel * const CommentsModel_;
		SortCommentsProxyModel * const ProxyModel_;
		QHash<QStandardItem*, CommentEntry> Item2RecentComment_;
		QSet<CommentEntry> RecentComments_;
	public:
		struct CommentID
		{
			QByteArray AccountID_;
			qint64 CommentID_;

			CommentID ()
			: CommentID_ (-1)
			{}

			bool operator== (const CommentID& otherComment) const
			{
				return CommentID_ == otherComment.CommentID_ &&
				AccountID_ == otherComment.AccountID_;
			}
		};
		typedef QList<CommentID> CommentIDs_t;
	private:
		QSet<CommentID> ReadComments_;

	public:
		CommentsWidget (QWidget *parent = 0);

		QString GetName () const;
		CommentEntry GetRecentCommentFromIndex (const QModelIndex& index) const;
	private:
		void FillModel ();
		void AddItemsToModel (const QList<CommentEntry>& comments);
		CommentEntry GetComment (const QString& accountId, int commentId) const;

	private slots:
		void handleLinkActivated (const QString& url);
		void handleDeleteComment (const QString& accountId, int commentId);
		void handleMarkCommentAsRead (const QString& accountId, int commentId);
		void handleAddComment (const QString& accountId, int entryId, int commentId);
		void handleCommentsUpdated ();
	};

	QDataStream& operator>> (QDataStream& in, LC::Blogique::CommentsWidget::CommentID& comment);
	QDataStream& operator<< (QDataStream& out, const LC::Blogique::CommentsWidget::CommentID& comment);
	uint qHash (const LC::Blogique::CommentsWidget::CommentID& cid);
}
}

Q_DECLARE_METATYPE (LC::Blogique::CommentsWidget::CommentIDs_t)
