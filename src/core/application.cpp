/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "application.h"
#include <typeinfo>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/scoped_array.hpp>
#include <QEvent>
#include <QtDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QMetaType>
#include <QModelIndex>
#include <QSessionManager>
#include <QInputDialog>
#include <QSplashScreen>
#include <QProcess>
#include <QTimer>
#include <QCryptographicHash>
#include <QTextCodec>
#include <QtQml>
#include <QStyle>

#ifdef Q_OS_MAC
#include <QProxyStyle>
#include <QStyleFactory>
#endif

#ifndef Q_OS_WIN32
#include <unistd.h>
#include <fcntl.h>
#endif

#include <interfaces/isyncable.h>
#include <interfaces/ihaveshortcuts.h>
#include <util/util.h>
#include <util/structuresops.h>
#include <util/sys/paths.h>
#include <util/qml/tooltipitem.h>
#include <util/threads/concurrentexception.h>

#ifdef WITH_QWT
#include <util/qml/plotitem.h>
#endif

#include "debugmessagehandler.h"
#include "tagsmanager.h"
#include "mainwindow.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "coreinstanceobject.h"
#include "rootwindowsmanager.h"
#include "splashscreen.h"
#include "config.h"

#ifdef Q_OS_WIN32
#include "winwarndialog.h"
#endif

namespace bpo = boost::program_options;

namespace LC
{
	Application::Application (int& argc, char **argv)
	: QApplication (argc, argv)
	, DefaultSystemStyleName_ (style ()->objectName ())
	{
		/* Workaround for
		 * https://code.google.com/p/libproxy/issues/detail?id=197
		 * https://bugzilla.novell.com/show_bug.cgi?id=866692
		 */
		qputenv ("PX_MODULE_PATH", {});

		Arguments_ = arguments ().mid (1);
		bpo::options_description desc ("Allowed options");

		{
			std::vector<std::wstring> strings;
			for (const auto& arg : Arguments_)
				strings.push_back (arg.toStdWString ());
			bpo::wcommand_line_parser parser (strings);
			VarMap_ = Parse (parser, &desc);
		}

		if (VarMap_.count ("help"))
		{
			std::cout << "LeechCraft " << LEECHCRAFT_VERSION << " (https://leechcraft.org)" << std::endl;
			std::cout << std::endl;
			std::cout << desc << std::endl;
			std::exit (EHelpRequested);
		}

		if (VarMap_.count ("version"))
		{
			std::cout << "LeechCraft " << LEECHCRAFT_VERSION << " (https://leechcraft.org)" << std::endl;
#ifdef Q_OS_WIN32
			std::cout << " <this version does not have UNLIMITED CAT POWER :(>" << std::endl;
#else
			std::cout << " this version has the UNLIMITED CAT POWER :3 ε:" << std::endl;
#endif
			std::exit (EVersionRequested);
		}

		if (VarMap_.count ("no-app-catch"))
			CatchExceptions_ = false;

		if (VarMap_.count ("clrsckt"))
			QLocalServer::removeServer (GetSocketName ());

		if (VarMap_.count ("no-resource-caching"))
			setProperty ("NoResourceCaching", true);

		if (VarMap_.count ("restart"))
		{
			Arguments_.removeAll ("--restart");
			EnterRestartMode ();
			return;
		}

		// Sanity checks
		if (!VarMap_.count ("plugin") && IsAlreadyRunning ())
			std::exit (EAlreadyRunning);

		Util::InstallTranslator ("", "qt", "qt5");

		QDir home = QDir::home ();
		if (!home.exists (".leechcraft"))
			if (!home.mkdir (".leechcraft"))
			{
				QMessageBox::critical (0,
						"LeechCraft",
						QDir::toNativeSeparators (tr ("Could not create path %1/.leechcraft")
							.arg (QDir::homePath ())));
				std::exit (EPaths);
			}

		// Things are sane, prepare
		QCoreApplication::setApplicationName ("Leechcraft");
		QCoreApplication::setApplicationVersion (LEECHCRAFT_VERSION);
		QCoreApplication::setOrganizationName ("Deviant");

		Util::InstallTranslator ("");

		qRegisterMetaType<QModelIndex> ("QModelIndex");
		qRegisterMetaType<QModelIndex*> ("QModelIndexStar");
		qRegisterMetaType<TagsManager::TagsDictionary_t> ("LC::TagsManager::TagsDictionary_t");
		qRegisterMetaType<Util::QtException_ptr> ("LC::Util::QtException_ptr");
		qRegisterMetaType<Entity> ("LC::Entity");
		qRegisterMetaType<Entity> ("Entity");
		qRegisterMetaType<IHookProxy_ptr> ("LC::IHookProxy_ptr");
		qRegisterMetaType<QKeySequences_t> ("QKeySequences_t");
		qRegisterMetaTypeStreamOperators<QKeySequences_t> ("QKeySequences_t");
		qRegisterMetaTypeStreamOperators<TagsManager::TagsDictionary_t> ("LC::TagsManager::TagsDictionary_t");
		qRegisterMetaTypeStreamOperators<Entity> ("LC::Entity");

		qmlRegisterType<Util::ToolTipItem> ("org.LC.common", 1, 0, "ToolTip");

		qRegisterMetaType<QList<QPointF>> ("QList<QPointF>");
#ifdef WITH_QWT
		qmlRegisterType<Util::PlotItem> ("org.LC.common", 1, 0, "Plot");
#else
		qmlRegisterUncreatableType<QObject> ("org.LC.common", 1, 0, "Plot",
				"LeechCraft core has been built without Qwt support, Plot item is not available.");
#endif

		ParseCommandLine ();

		if (!VarMap_.count ("nolog"))
		{
			// Say hello to logs
			qDebug () << "======APPLICATION STARTUP======";
			qWarning () << "======APPLICATION STARTUP======";
		}

#ifdef Q_OS_WIN32
		new WinWarnDialog;
#endif

		CheckStartupPass ();

		Core::Instance ();
		InitSettings ();

		InitPluginsIconset ();

		setWindowIcon (QIcon ("lcicons:/resources/images/leechcraft.svg"));

		setQuitOnLastWindowClosed (false);

		Splash_ = new SplashScreen { QPixmap (":/resources/images/splash.svg"), Qt::SplashScreen };
		Splash_->setUpdatesEnabled (true);
		if (!VarMap_.count ("no-splash-screen"))
		{
			Splash_->show ();
			Splash_->repaint ();
		}

		QTimer::singleShot (50,
				this,
				SLOT (finishInit ()));

		InitSessionManager ();
	}

	const QStringList& Application::Arguments () const
	{
		return Arguments_;
	}

	bpo::variables_map Application::Parse (bpo::wcommand_line_parser& parser,
			bpo::options_description *desc) const
	{
		bpo::variables_map vm;
		bpo::options_description invisible ("Invisible options");
		invisible.add_options ()
				("entity,E", bpo::wvalue<std::vector<std::wstring>> (), "the entity to handle");

		desc->add_options ()
				("help", "show the help message")
				("version,v", "print LC version")
				("download,D", "only choose downloaders for the entity: it should be downloaded but not handled")
				("handle,H", "only choose handlers for the entity: it should be handled but not downloaded")
				("type,T", bpo::value<std::string> (), "the type of the entity: url, url_encoded, file (for file paths) and such")
				("additional,A", bpo::value<std::vector<std::string>> (), "parameters for the Additional entity map in the form of name:value")
				("automatic", "the entity is a result of some automatic stuff, not user's actions")
				("bt", "print backtraces for warning messages into warning.log")
				("plugin,P", bpo::value<std::vector<std::string>> (), "load only given plugin and ignore already running instances of LC")
				("multiprocess,M", "load plugins in separate processes")
				("nolog", "disable custom file logger and print everything to stdout/stderr")
				("clrsckt", "clear stalled socket, use if you believe previous LC instance has terminated but failed to close its local socket properly")
				("no-app-catch", "disable exceptions catch-all in QApplication::notify(), useful for debugging purposes")
				("safe-mode", "disable all plugins so that you can manually enable them in Settings later")
				("list-plugins", "list all non-adapted plugins that were found and exit (this one doesn't check if plugins are valid and loadable)")
				("no-resource-caching", "disable caching of dynamic loadable resources (useful for stuff like Azoth themes development)")
				("autorestart", "automatically restart LC if it's closed (not guaranteed to work everywhere, especially on Windows and Mac OS X)")
				("minimized", "start LC minimized to tray")
				("no-splash-screen", "do not show the splash screen")
				("restart", "restart the LC");
		bpo::positional_options_description pdesc;
		pdesc.add ("entity", -1);

		bpo::options_description all;
		all.add (*desc);
		all.add (invisible);

		bpo::store (parser
				.options (all)
				.positional (pdesc)
				.allow_unregistered ()
				.run (), vm);
		bpo::notify (vm);

		return vm;
	}

	const bpo::variables_map& Application::GetVarMap () const
	{
		return VarMap_;
	}

#ifdef Q_OS_WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

	QString Application::GetSocketName ()
	{
		QString templ = QString ("LeechCraft_local_socket_%1");
#ifdef Q_OS_WIN32
		boost::scoped_array<TCHAR> buffer (new TCHAR [0]);
		DWORD size = 0;
		GetUserName (buffer.get (), &size);
		buffer.reset (new TCHAR [size]);
		if (GetUserName (buffer.get (), &size))
#ifdef _UNICODE
			return templ.arg (QString::fromWCharArray (buffer.get ()));
#else
			return templ.arg (buffer.get ());
#endif
		else
			return templ.arg ("unknown");
#else
		return templ.arg (getuid ());
#endif
	}

	SplashScreen* Application::GetSplashScreen () const
	{
		return Splash_;
	}

	void Application::InitiateRestart ()
	{
		QStringList arguments = Arguments_;
		arguments << "--restart";
		QProcess::startDetached (applicationFilePath (), arguments);

		Quit ();
	}

	void Application::Quit ()
	{
		Core::Instance ().Release ();
	}

	bool Application::notify (QObject *obj, QEvent *event)
	{
		if (event->type () == QEvent::LanguageChange)
			return true;

		if (!CatchExceptions_)
			return QApplication::notify (obj, event);

		try
		{
			return QApplication::notify (obj, event);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< QString::fromUtf8 (e.what ())
				<< typeid (e).name ()
				<< "for"
				<< obj
				<< event
				<< event->type ();
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< obj
				<< event
				<< event->type ();
		}
		return false;
	}

	bool Application::IsAlreadyRunning () const
	{
		QLocalSocket socket;
		socket.connectToServer (GetSocketName ());
		if (socket.waitForConnected () ||
				socket.state () == QLocalSocket::ConnectedState)
		{
			QDataStream out (&socket);
			out << Arguments_ << QDir::currentPath ();
			if (socket.waitForBytesWritten ())
				return true;
			if (socket.error() == QLocalSocket::UnknownSocketError)
				return true;
		}
		else
		{
			switch (socket.error ())
			{
				case QLocalSocket::ServerNotFoundError:
				case QLocalSocket::ConnectionRefusedError:
					break;
				default:
					qWarning () << Q_FUNC_INFO
						<< "socket error"
						<< socket.error ();
					return true;
			}
		}

		// Clear any halted servers and their messages
		QLocalServer::removeServer (GetSocketName ());
		return false;
	}

	namespace
	{
		void Rotate (QDir lcDir, const QString& filename)
		{
			if (!lcDir.exists (filename))
				return;

			if (QFileInfo (lcDir.filePath (filename)).size () < 20 * 1024 * 1024)
				return;

			if (lcDir.exists (filename + ".0") &&
					!lcDir.remove (filename + ".0"))
				qWarning () << Q_FUNC_INFO
						<< "unable to remove old file"
						<< filename + ".0";

			if (!lcDir.rename (filename, filename + ".0"))
				qWarning () << Q_FUNC_INFO
						<< "failed to rename"
						<< filename
						<< "to"
						<< filename + ".0";
		}
	}

	void Application::ParseCommandLine ()
	{
		static const auto flags = [this]
		{
			DebugHandler::DebugWriteFlags flags = DebugHandler::DWFNone;
			if (VarMap_.count ("nolog"))
				flags |= DebugHandler::DWFNoFileLog;
			if (VarMap_.count ("bt"))
				flags |= DebugHandler::DWFBacktrace;
			return flags;
		} ();

		qInstallMessageHandler ([] (QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
				{
					DebugHandler::Write (type, ctx, msg.toLocal8Bit ().constData (), flags);
				});

		QDir lcDir = QDir::home ();
		lcDir.cd (".leechcraft");

		Rotate (lcDir, "debug.log");
		Rotate (lcDir, "warning.log");
	}

	void Application::InitPluginsIconset ()
	{
		QStringList paths;

		auto appendPaths = [&paths] (const QString& iconset)
		{
			const auto& pluginsPath = "global_icons/plugins/" + iconset;
			for (const auto& cand : Util::GetPathCandidates (Util::SysPath::Share, pluginsPath))
				paths.append (cand);
		};

		const auto& pluginsIconset = XmlSettingsManager::Instance ()->property ("PluginsIconset").toString ();
		if (pluginsIconset != "Default")
			appendPaths (pluginsIconset);

		appendPaths ("default");

		paths.append (":/");

		QDir::setSearchPaths ("lcicons", paths);
	}

	void Application::EnterRestartMode ()
	{
		QTimer *timer = new QTimer;
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (checkStillRunning ()));
		timer->start (1000);
	}

	void Application::CheckStartupPass ()
	{
		const auto& storedPass = XmlSettingsManager::Instance ()->property ("StartupPassword").toString ();
		if (storedPass.isEmpty ())
			return;

		const auto& pass = QInputDialog::getText (0,
				tr ("Startup password"),
				tr ("Enter startup password for LeechCraft:"),
				QLineEdit::Password);
		if (QCryptographicHash::hash (pass.toUtf8 (), QCryptographicHash::Sha1).toHex () != storedPass)
		{
			if (!pass.isEmpty ())
				QMessageBox::critical (0, "LeechCraft", tr ("Sorry, incorrect password"));
			std::exit (0);
		}
	}

	void Application::InitSettings ()
	{
		XmlSettingsManager::Instance ()->RegisterObject ("AppQStyle",
				this, "handleAppStyle");
		handleAppStyle ();

		XmlSettingsManager::Instance ()->RegisterObject ("Language",
				this, "handleLanguage");
		PreviousLangName_ = XmlSettingsManager::Instance ()->property ("Language").toString ();
	}

	void Application::InitSessionManager ()
	{
		setFallbackSessionManagementEnabled (false);

		connect (this,
				&QGuiApplication::saveStateRequest,
				this,
				[this] (QSessionManager& sm)
				{
					if (Arguments_.contains ("-autorestart"))
						sm.setRestartHint (QSessionManager::RestartImmediately);
				});
	}

	void Application::finishInit ()
	{
		auto rwm = Core::Instance ().GetRootWindowsManager ();
		rwm->Initialize ();
		Core::Instance ().DelayedInit ();

		const auto win = rwm->GetMainWindow (0);
		win->showFirstTime ();
		Splash_->finish (win);
	}

#ifdef Q_OS_MAC
	namespace
	{
		class ToolbarFixerProxy : public QProxyStyle
		{
		public:
			ToolbarFixerProxy (QStyle *other)
			: QProxyStyle (other)
			{
			}

			int pixelMetric (PixelMetric metric, const QStyleOption *opt, const QWidget *w) const
			{
				auto result = baseStyle ()->pixelMetric (metric, opt, w);
				if (metric == PM_ToolBarIconSize)
					result = std::min (24, result);
				return result;
			}
		};
	}
#endif

	void Application::handleAppStyle ()
	{
		auto style = XmlSettingsManager::Instance ()->property ("AppQStyle").toString ();

		if (style == "Default")
			style = DefaultSystemStyleName_;

		if (style.isEmpty ())
		{
#ifdef Q_OS_WIN32
			style = "Plastique";
			XmlSettingsManager::Instance ()->setProperty ("AppQStyle", style);
#endif
		}

#ifdef Q_OS_MAC
		if (auto styleObj = QStyleFactory::create (style))
			setStyle (new ToolbarFixerProxy (styleObj));
#else
		setStyle (style);
#endif
	}

	void Application::handleLanguage ()
	{
		const auto& newLang = XmlSettingsManager::Instance ()->property ("Language").toString ();
		if (newLang == PreviousLangName_)
			return;

		PreviousLangName_ = newLang;

		if (QMessageBox::question (0,
					"LeechCraft",
					tr ("This change requires restarting LeechCraft. "
						"Do you want to restart now?"),
					QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
			return;

		InitiateRestart ();
	}

	void Application::checkStillRunning ()
	{
		if (IsAlreadyRunning ())
			return;

#ifndef Q_OS_WIN32
		const auto fdlimit = sysconf (_SC_OPEN_MAX);
		for (int i = STDERR_FILENO + 1; i < fdlimit; ++i)
			fcntl (i, F_SETFD, FD_CLOEXEC);
#endif

		QProcess::startDetached (applicationFilePath (), Arguments_);

		Quit ();
	}
}
