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

#include <QDialog>
#include "ui_polldialog.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class PollDialog : public QDialog
	{
		Q_OBJECT

		Ui::PollDialog Ui_;

		QStandardItemModel *CheckModel_;
		QStandardItemModel *RadioModel_;
		QStandardItemModel *DropModel_;
		QStandardItemModel *PollTypeModel_;
		bool ItemIsChanged_ = false;
		QHash<QString, QString> Type2Question_;

		enum PollCanView
		{
			ViewAll,
			ViewOnlyFriends,
			ViewOnlyMe
		};

		enum PollCanVote
		{
			VoteAll,
			VoteOnlyFriends
		};

	public:
		enum PollType
		{
			CheckBoxes,
			RadioButtons,
			DropdownBox,
			TextEntry,
			Scale
		};

		explicit PollDialog (QWidget *parent = 0);

		QString GetPollName () const;
		QString GetWhoCanView () const;
		QString GetWhoCanVote () const;
		QStringList GetPollTypes () const;
		QString GetPollQuestion (const QString& type) const;
		QVariantMap  GetPollFields (const QString& pollType) const;
		int GetScaleFrom () const;
		int GetScaleTo () const;
		int GetScaleBy () const;
		int GetTextSize () const;
		int GetTextMaxLength () const;

		void accept ();
	private:
		QVariantMap GetFields (QStandardItemModel *model) const;
		bool IsScaleValuesAreValid () const;

	private slots:
		void on_AddField__released ();
		void on_RemoveField__released ();
		void on_PollType__currentIndexChanged (int index);
		void handleItemActivated (const QModelIndex& index);
		void handleItemChanged (QStandardItem *item);
		void on_PollQuestion__textChanged (const QString& text);
	};
}
}
}
