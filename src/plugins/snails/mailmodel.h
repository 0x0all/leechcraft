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

#include <memory>
#include <QStringList>
#include <QAbstractItemModel>
#include <QList>
#include "messagelistactioninfo.h"

namespace LC
{
namespace Snails
{
	class MessageListActionsManager;
	struct MessageInfo;

	class MailModel : public QAbstractItemModel
	{
		Q_OBJECT

		const MessageListActionsManager * const ActionsMgr_;

		const QStringList Headers_;

		QStringList Folder_;

		struct TreeNode;
		typedef std::shared_ptr<TreeNode> TreeNode_ptr;
		typedef std::weak_ptr<TreeNode> TreeNode_wptr;
		const TreeNode_ptr Root_;

		QHash<QByteArray, QList<TreeNode_ptr>> FolderId2Nodes_;
		QHash<QByteArray, QByteArray> MsgId2FolderId_;

		mutable QHash<QByteArray, QList<MessageListActionInfo>> MsgId2Actions_;
	public:
		enum class Column
		{
			From,
			UnreadChildren,
			StatusIcon,
			Subject,
			Date
		};
		static const Column MaxColumn = Column::Date;

		enum MailRole
		{
			ID = Qt::UserRole + 1,
			Sort,
			IsRead,
			UnreadChildrenCount,
			TotalChildrenCount,
			MessageActions,
			MsgInfo
		};

		MailModel (const MessageListActionsManager*, QObject* = 0);

		QVariant headerData (int, Qt::Orientation, int) const override;
		int columnCount (const QModelIndex& = {}) const override;
		QVariant data (const QModelIndex&, int) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QModelIndex index (int, int, const QModelIndex& = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = {}) const override;
		bool setData (const QModelIndex&, const QVariant&, int) override;

		QStringList mimeTypes () const override;
		QMimeData* mimeData (const QModelIndexList&) const override;
		Qt::DropActions supportedDragActions () const override;

		void SetFolder (const QStringList&);
		QStringList GetCurrentFolder () const;

		void Clear ();

		void Append (QList<MessageInfo>);
		bool Remove (const QByteArray&);

		void UpdateReadStatus (const QList<QByteArray>& msgIds, bool read);

		void MarkUnavailable (const QList<QByteArray>&);

		QList<QByteArray> GetCheckedIds () const;
		bool HasCheckedIds () const;
	private:
		void UpdateParents (const QByteArray&, bool);

		void RemoveNode (const TreeNode_ptr&);
		bool AppendStructured (const MessageInfo&);

		void EmitRowChanged (const TreeNode_ptr&);

		QModelIndex GetIndex (const TreeNode_ptr& node, int column) const;
		QList<QModelIndex> GetIndexes (const QByteArray& folderId, int column) const;
		QList<QList<QModelIndex>> GetIndexes (const QByteArray& folderId, const QList<int>& columns) const;
	signals:
		void messageListUpdated ();
		void messagesSelectionChanged ();
	};
}
}
