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

#include <QWidget>
#include <interfaces/blogique/iblogiquesidewidget.h>
#include <interfaces/blogique/ipostoptionswidget.h>
#include <interfaces/media/audiostructs.h>
#include "ui_postoptionswidget.h"

namespace Media
{
	class ICurrentSongKeeper;
}

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class LJAccount;

	class PostOptionsWidget : public QWidget
							, public IBlogiqueSideWidget
							, public IPostOptionsWidget
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Blogique::IBlogiqueSideWidget
				LeechCraft::Blogique::IPostOptionsWidget)

		Ui::PostOptions Ui_;
		LJAccount *Account_;
		quint32 AllowMask_;
	public:
		PostOptionsWidget (QWidget *parent = 0);

		QString GetName () const;
		SideWidgetType GetWidgetType () const;
		QVariantMap GetPostOptions () const;
		void SetPostOptions (const QVariantMap& map);
		QVariantMap GetCustomData () const;
		void SetCustomData (const QVariantMap& map);
		void SetAccount (QObject *account);

		QStringList GetTags () const;
		void SetTags (const QStringList& tags);
		QDateTime GetPostDate () const;
		void SetPostDate (const QDateTime& date);
	private:
		void FillItems ();

	public slots:
		void handleAutoUpdateCurrentMusic ();
	private slots:
		void on_CurrentTime__released ();
		void on_Access__activated (int index);
		void on_UserPic__currentIndexChanged (int index);
		void on_AutoDetect__released ();
		void handleCurrentSongChanged (const Media::AudioInfo& ai);
	};
}
}
}

