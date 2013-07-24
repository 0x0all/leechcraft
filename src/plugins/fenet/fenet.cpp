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

#include "fenet.h"
#include <QIcon>
#include <QApplication>
#include <QProcess>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "wmfinder.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Fenet
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Finder_ = new WMFinder;

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "fenetsettings.xml");

		XSD_->SetDataSource ("SelectedWM", Finder_->GetFoundModel ());

		if (!QApplication::arguments ().contains ("--desktop"))
			return;

		Process_ = new QProcess (this);
		connect (Process_,
				SIGNAL (error (QProcess::ProcessError)),
				this,
				SLOT (handleProcessError ()));

		StartWM ();

		XmlSettingsManager::Instance ().RegisterObject ("SelectedWM", this, "restartWM");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Fenet";
	}

	void Plugin::Release ()
	{
		KillWM ();
	}

	QString Plugin::GetName () const
	{
		return "Fenet";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Window manager control plugin.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::StartWM ()
	{
		const auto& found = Finder_->GetFound ();
		if (found.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no known WMs are found, aborting";
			return;
		}

		auto selected = XmlSettingsManager::Instance ()
				.property ("SelectedWM").toString ();

		auto pos = std::find_if (found.begin (), found.end (),
				[&selected] (const WMInfo& info) { return info.Name_ == selected; });
		if (pos == found.end ())
			pos = found.begin ();

		const auto& session = pos->Session_;
		qDebug () << Q_FUNC_INFO << "starting" << session;
		Process_->start (session);
	}

	void Plugin::KillWM ()
	{
		if (!Process_)
			return;

		Process_->terminate ();
		if (Process_->state () != QProcess::NotRunning && !Process_->waitForFinished (3000))
			Process_->kill ();
	}

	void Plugin::restartWM ()
	{
		KillWM ();
		StartWM ();
	}

	void Plugin::handleProcessError ()
	{
		qWarning () << Q_FUNC_INFO
				<< "process error:"
				<< Process_->error ()
				<< Process_->errorString ()
				<< Process_->exitCode ()
				<< Process_->exitStatus ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_fenet, LeechCraft::Fenet::Plugin);
