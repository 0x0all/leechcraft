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
#include <boost/optional.hpp>
#include <QObject>
#include <util/db/oral/oralfwd.h>
#include <util/db/oral/oraltypes.h>

namespace LeechCraft
{
namespace NamAuth
{
	class SQLStorageBackend : public QObject
	{
		Q_OBJECT

		std::shared_ptr<QSqlDatabase> DB_;
	public:
		struct AuthRecord
		{
			QString RealmName_;
			QString Context_;
			QString Login_;
			QString Password_;

			static QByteArray ClassName ()
			{
				return "AuthRecords";
			}

			static QByteArray FieldNameMorpher (const QByteArray& str)
			{
				return str;
			}

			using Constraints = Util::oral::Constraints<
					Util::oral::PrimaryKey<0, 1>
				>;
		};
	private:
		Util::oral::ObjectInfo_ptr<AuthRecord> AdaptedRecord_;
	public:
		SQLStorageBackend ();

		static QString GetDBPath ();

		boost::optional<AuthRecord> GetAuth (const QString&, const QString&);
		void SetAuth (const AuthRecord&);
	};
}
}
