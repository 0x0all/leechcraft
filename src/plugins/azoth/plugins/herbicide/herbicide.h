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

#ifndef PLUGINS_AZOTH_PLUGINS_HERBICIDE_HERBICIDE_H
#define PLUGINS_AZOTH_PLUGINS_HERBICIDE_HERBICIDE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>

namespace LeechCraft
{
namespace Azoth
{
class IMessage;

namespace Herbicide
{
	class ConfWidget;
	class Logger;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Herbicide")

		Util::XmlSettingsDialog_ptr SettingsDialog_;
		ConfWidget *ConfWidget_;

		Logger *Logger_;

		QSet<QObject*> AskedEntries_;
		QSet<QObject*> AllowedEntries_;
		QSet<IMessage*> OurMessages_;

		QHash<QObject*, QString> DeniedAuth_;

		QSet<QRegExp> Whitelist_;
		QSet<QRegExp> Blacklist_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	private:
		bool IsConfValid () const;
		bool IsEntryAllowed (QObject*) const;

		void ChallengeEntry (IHookProxy_ptr, QObject*);
		void GreetEntry (QObject*);
	public slots:
		void hookGotAuthRequest (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry,
				QString msg);
		void hookGotMessage (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
	private slots:
		void handleWhitelistChanged ();
		void handleBlacklistChanged ();
	};
}
}
}

#endif
