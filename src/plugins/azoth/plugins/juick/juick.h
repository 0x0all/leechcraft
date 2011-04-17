/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_JUICK_JUICK_H
#define PLUGINS_AZOTH_PLUGINS_JUICK_JUICK_H

#include <QObject>
#include <QRegExp>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Juick
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)
		
		QRegExp UserRX_;
		QRegExp PostRX_;
		QRegExp IdRX_;
		QRegExp UnsubRX_;
		QRegExp ReplyRX_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QStringList Provides () const;
		QStringList Needs () const;
		QStringList Uses () const;
		void SetProvider (QObject*, const QString&);

		QSet<QByteArray> GetPluginClasses () const;
	private:
		QString FormatBody (QString);
		bool ShouldHandle (QObject* msgObj, int direction, int type);
	public slots:
		void hookFormatBodyEnd (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QString body,
				QObject *message);
		void hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,  
				QObject *chatTab,
				QObject *entry,
				int type, 
				QString variant, 
				QString text);

	};
}
}
}

#endif

