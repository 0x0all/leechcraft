/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin  <MaledictusDeMagog@gmail.com>
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

#include "exportwizard.h"
#include <QFileDialog>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QPrinter>
#include <QWebFrame>
#include <QWebView>
#include "interfaces/blogique/ibloggingplatform.h"
#include <interfaces/core/ientitymanager.h>
#include <util/util.h>
#include "core.h"

namespace LeechCraft
{
namespace Blogique
{
	ExportWizard::ExportWizard(QWidget *parent)
	: QWizard (parent)
	, AllTagsModel_ (new QStandardItemModel (this))
	, SelectedTagsModel_ (new QStandardItemModel (this))
	, Formats_ (new QButtonGroup (this))
	{
		Ui_.setupUi (this);

		connect (this,
				SIGNAL (currentIdChanged (int)),
				this,
				SLOT(handleCurrentIdChanged (int)));

		connect (Ui_.AccountSelection_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleAccountChanged (int)));

		for (auto acc : Core::Instance ().GetAccounts ())
		{
			if (auto bp = qobject_cast<IBloggingPlatform*> (acc->GetParentBloggingPlatform ()))
			{
				Ui_.AccountSelection_->addItem (bp->GetBloggingPlatformIcon (),
						acc->GetAccountName ());
				Id2Account_ [Ui_.AccountSelection_->count () - 1] = acc;

				connect (acc->GetQObject (),
						SIGNAL (tagsUpdated (QHash<QString, int>)),
						this,
						SLOT (handleTagsUpdated (QHash<QString, int>)));
				acc->RequestTags ();
			}
		}

		int i = 0;
		for (auto btn : Ui_.OutputFormatPage_->findChildren<QRadioButton*> ())
		{
			Id2RadioButton_ [i] = btn;
			Formats_->addButton (btn, i++);
		}
		Ui_.PlainText_->setProperty ("ExportFormat", ExportFormat::PlainText);
		Ui_.Html_->setProperty ("ExportFormat", ExportFormat::Html);
		Ui_.Fb2_->setProperty ("ExportFormat", ExportFormat::Fb2);
		Ui_.Pdf_->setProperty ("ExportFormat", ExportFormat::Pdf);

		Ui_.FromDate_->setCalendarPopup (true);
		Ui_.TillDate_->setCalendarPopup (true);
		Ui_.TillDate_->setDateTime (QDateTime::currentDateTime ());

		Ui_.AllTagsView_->setModel (AllTagsModel_);
		Ui_.AllTagsView_->setHeaderHidden (true);
		Ui_.SelectedTagsView_->setModel (SelectedTagsModel_);
		Ui_.SelectedTagsView_->setHeaderHidden (true);
		connect (Ui_.AddTag_,
				SIGNAL(released ()),
				this,
				SLOT (addTag ()));
		connect (Ui_.RemoveTag_,
				SIGNAL(released ()),
				this,
				SLOT (removeTag()));

		connect (Ui_.SelectPath_,
				SIGNAL (released ()),
				this,
				SLOT (selectExportPath ()));
	}

	void ExportWizard::FillTags (IAccount *acc)
	{
		if (auto rc = AllTagsModel_->rowCount ())
			AllTagsModel_->removeRows (0, rc);

		for (const auto& tag : Account2Tags_.value (acc))
		{
			auto item = new QStandardItem (tag);
			item->setEditable (false);
			AllTagsModel_->appendRow (item);
		}
	}

	bool ExportWizard::validateCurrentPage ()
	{
		switch (currentId ())
		{
		case WelcomPage:
			if (Ui_.AccountSelection_->currentIndex () == -1)
			{
				QMessageBox::warning (this,
						"LechCraft",
						tr ("You should select an account to export"));
				return false;
			}
			return true;
		case FormatPage:
			if (Ui_.SavePath_->text ().isEmpty ())
			{
				QMessageBox::warning (this,
					"LeechCraft",
					tr ("You should enter export path"));
				return false;
			}
			return true;
		case ContentPage:
			if (Ui_.WithDateRange_->isChecked () &&
					(Ui_.FromDate_->dateTime () > Ui_.TillDate_->dateTime ()))
			{
				QMessageBox::warning (this,
						"LeechCraft",
						tr ("Invalid date range"));
				return false;
			}

			if (Ui_.SelectedTags_->isChecked () &&
					!SelectedTagsModel_->rowCount ())
			{
				QMessageBox::warning (this,
						"LeechCraft",
						tr ("At least one tag should be selected"));
				return false;
			}
			return true;
		case OverviewPage:
		case ExportPage:
			Ui_.ExportProgress_->setMinimum (0);
			Ui_.ExportProgress_->setMaximum (0);
			return true;
		}

		return true;
	}

	void ExportWizard::reject ()
	{
		//TODO reject export
		QDialog::reject ();
	}

	void ExportWizard::handleAccountChanged (int index)
	{
		if (index == -1)
			return;

		FillTags (Id2Account_.value (index));
	}

	void ExportWizard::handleCurrentIdChanged (int id)
	{
		switch (id)
		{
			case WelcomPage:
			case FormatPage:
			case ContentPage:
				Ui_.FromDate_->setDate (QDate::fromString ("01.01.1970", "dd.MM.yyyy"));
				Ui_.TillDate_->setDate (QDate::currentDate ());
			default:
				return;
			case OverviewPage:
			{
				Ui_.AccountLabel_->setText (Ui_.AccountSelection_->currentText ());
				Ui_.FormatLabel_->setText (Formats_->checkedButton ()->text ());
				Ui_.EntriesLabel_->setText (Ui_.AllEntries_->isChecked () ?
					tr ("All entries") :
					tr ("Only between %1 and %2")
							.arg (Ui_.FromDate_->text ())
							.arg (Ui_.TillDate_->text ()));
				QStringList selectedTags;
				for (int i = 0; Ui_.SelectedTags_->isChecked () && i < SelectedTagsModel_->rowCount ();
						++i)
					 selectedTags << SelectedTagsModel_->index (i, 0).data ().toString ();
				Ui_.EntriesTagsLabel_->setText (Ui_.AllTags_->isChecked () ?
							tr ("All tags") :
							tr ("Only tags: %1")
						.arg (selectedTags.join (", ")));
				Ui_.SavePathLabel_->setText (Ui_.SavePath_->text ());
				break;
			}
			case ExportPage:
			{
				if (!Id2Account_.contains (Ui_.AccountSelection_->currentIndex ()))
					return;

				Filter filter;
				filter.CustomDate_ = Ui_.WithDateRange_->isChecked ();
				filter.BeginDate_ = Ui_.FromDate_->dateTime ();
				filter.EndDate_ = Ui_.TillDate_->dateTime ();
				QStringList selectedTags;
				for (int i = 0; Ui_.SelectedTags_->isChecked () && i < SelectedTagsModel_->rowCount ();
					 ++i)
					 selectedTags << SelectedTagsModel_->index (i, 0).data ().toString ();
				filter.Tags_ = Ui_.SelectedTags_->isChecked () ? selectedTags : QStringList ();

				auto account = Id2Account_ [Ui_.AccountSelection_->currentIndex ()];
				connect (account->GetQObject (),
						SIGNAL (gotFilteredEntries (QList<Entry>)),
						this,
						SLOT (handleGotFilteredEntries (QList<Entry>)));
				connect (account->GetQObject (),
						SIGNAL (gettingFilteredEntriesFinished ()),
						this,
						SLOT (handleGettingFilteredEntriesFinished ()));
				account->GetEntriesWithFilter (filter);
				break;
			}
		}
	}

	void ExportWizard::selectExportPath ()
	{
		const auto& path = QFileDialog::getSaveFileName (this,
				"LeechCraft",
				QDir::homePath ());

		Ui_.SavePath_->setText (path);
	}

	void ExportWizard::addTag ()
	{
		auto srcIndex = Ui_.AllTagsView_->selectionModel ()->selectedRows ().value (0);
		if (!srcIndex.isValid ())
			return;

		auto row = AllTagsModel_->takeRow (srcIndex.row ());
		SelectedTagsModel_->appendRow (row);
	}

	void ExportWizard::removeTag ()
	{
		auto srcIndex = Ui_.SelectedTagsView_->selectionModel ()->selectedRows ().value (0);
		if (!srcIndex.isValid ())
			return;

		auto row = SelectedTagsModel_->takeRow (srcIndex.row ());
		AllTagsModel_->appendRow (row);
	}

	void ExportWizard::handleTagsUpdated (const QHash<QString, int>& tags)
	{
		if (auto acc = qobject_cast<IAccount*> (sender ()))
		{
			const auto& tagsNames = tags.keys ();
			Account2Tags_ [acc] = tagsNames;

			if (acc != Id2Account_ [Ui_.AccountSelection_->currentIndex ()])
				return;

			FillTags (acc);
		}
	}

	namespace
	{
		QString GetHtmlContent (const QList<Entry>& entries)
		{
			QString content;
			QDateTime lastDate;
			for (const auto& entry : entries)
			{
				bool newDate = false;
				if (lastDate != entry.Date_)
				{
					lastDate = entry.Date_;
					newDate = true;
				}

				if (newDate)
					content += "<br><br><br><br><i>" + entry.Date_.toString (Qt::DefaultLocaleLongDate) + "</i><br><br>";

				content += "<b>" + entry.Subject_ + "</b><br><br>";
				content += entry.Content_ + "<br><br>";
				content += ("<b>Tags:</b><i>" + entry.Tags_.join (",") + "</i><br><br><br>");
			}

			return content;
		}

		void WritePlainText (const QList<Entry>& entries, const QString& filePath)
		{
			QFile file (filePath);
			if (!file.open (QIODevice::Append))
			{
				QMessageBox::warning (0,
						"LeechCraft",
						QObject::tr ("Unable to open file %1: %2")
								.arg (filePath)
								.arg (file.errorString ()));
				return;
			}

			QDateTime lastDate;
			QString content;
			QWebView wv;
			for (const auto& entry : entries)
			{
				bool newDate = false;
				if (lastDate != entry.Date_)
				{
					lastDate = entry.Date_;
					newDate = true;
				}

				if (newDate)
					content += "<br><br><br><br>" + entry.Date_.toString (Qt::DefaultLocaleLongDate) + "<br><br>";

				content += entry.Subject_ + "<br><br>";
				content += entry.Content_ + "<br><br>";
				content += ("Tags:" + entry.Tags_.join (",") + "<br><br><br>");
			}

			wv.setHtml (content);

			file.write (wv.page ()->currentFrame ()->toPlainText ().toUtf8 ());
			file.close ();
		}

		void WriteHtml (const QList<Entry>& entries, const QString& filePath)
		{

			QFile file (filePath);
			if (!file.open (QIODevice::Append))
			{
				QMessageBox::warning (0,
						"LeechCraft",
						QObject::tr ("Unable to open file %1: %2")
								.arg (filePath)
								.arg (file.errorString ()));
				return;
			}

			QWebView wv;
			wv.setHtml (GetHtmlContent (entries));

			file.write (wv.page ()->currentFrame ()->toHtml ().toUtf8 ());
			file.close ();
		}

		void WriteFb2 (const QList<Entry>& entries, const QString& filePath)
		{

		}

		void WritePdf (const QList<Entry>& entries, const QString& filePath)
		{
			QWebView wv;
			wv.setHtml (GetHtmlContent (entries));

			QPrinter printer (QPrinter::HighResolution);
			printer.setPaperSize (QPrinter::A4);
			printer.setOutputFormat (QPrinter::PdfFormat);

			printer.setOutputFileName(filePath);

			wv.print (&printer);
		}
	}

	void ExportWizard::handleGotFilteredEntries (const QList<Entry>& entries)
	{
		Entries_ << entries;
	}

	void ExportWizard::handleGettingFilteredEntriesFinished ()
	{
		switch (Formats_->checkedButton ()->property ("ExportFormat").toInt ())
		{
			case PlainText:
				WritePlainText (Entries_, Ui_.SavePath_->text ());
				break;
			case Html:
				WriteHtml (Entries_, Ui_.SavePath_->text ());
				break;
			case Fb2:
				WriteFb2 (Entries_, Ui_.SavePath_->text ());
				break;
			case Pdf:
				WritePdf (Entries_, Ui_.SavePath_->text ());
				break;
			default:
				return;
		}

		qDebug () << Q_FUNC_INFO << "got entries for export finished";
		Core::Instance ().GetCoreProxy ()->GetEntityManager ()->
				HandleEntity (Util::MakeNotification ("Blogique",
						tr ("Exporting finished"),
						Priority::PInfo_));
		deleteLater ();
	}

}
}
