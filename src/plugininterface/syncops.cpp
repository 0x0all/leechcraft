/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "syncops.h"
#include <QDataStream>
#include <QByteArray>
#include <QtDebug>

namespace LeechCraft
{
	namespace Sync
	{
		QDataStream& operator<< (QDataStream& out, const Payload& payload)
		{
			quint16 version = 1;
			out << version
					<< payload.Data_;
			return out;
		}

		QDataStream& operator>> (QDataStream& in, Payload& payload)
		{
			quint16 version;
			in >> version;
			switch (version)
			{
			case 1:
				in >> payload.Data_;
				break;
			default:
				qWarning () << Q_FUNC_INFO
						<< "unknown version"
						<< version;
			}
			return in;
		}

		QByteArray Serialize (const Payload& payload)
		{
			QByteArray result;

			{
				QDataStream str (&result, QIODevice::WriteOnly);
				str << payload;
			}
			return result;
		}

		Payload Deserialize (const QByteArray& data)
		{
			Payload result;

			QDataStream in (data);
			in >> result;
			return result;
		}
	}
}
