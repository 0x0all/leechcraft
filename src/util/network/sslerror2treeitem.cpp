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

#include "sslerror2treeitem.h"
#include <QStringList>
#include <QDateTime>
#include <QTreeWidgetItem>
#include <QSslError>

namespace LC
{
namespace Util
{
	QTreeWidgetItem* SslError2TreeItem (const QSslError& error)
	{
		const auto item = new QTreeWidgetItem { { "Error:", error.errorString () } };

		const auto& cer = error.certificate ();
		if (cer.isNull ())
		{
			new QTreeWidgetItem
			{
				item,
				{ QObject::tr ("Certificate"), QObject::tr ("(No certificate available for this error)") }
			};
			return item;
		}

		new QTreeWidgetItem
		{
			item,
			{
				QObject::tr ("Valid:"),
				!cer.isBlacklisted () ? QObject::tr ("yes") : QObject::tr ("no")
			}
		};
		new QTreeWidgetItem { item, { QObject::tr ("Effective date:"), cer.effectiveDate ().toString () } };
		new QTreeWidgetItem { item, { QObject::tr ("Expiry date:"), cer.expiryDate ().toString () } };
		new QTreeWidgetItem { item, { QObject::tr ("Version:"), cer.version () } };
		new QTreeWidgetItem { item, { QObject::tr ("Serial number:"), cer.serialNumber () } };
		new QTreeWidgetItem { item, { QObject::tr ("MD5 digest:"), cer.digest ().toHex () } };
		new QTreeWidgetItem { item, { QObject::tr ("SHA1 digest:"), cer.digest (QCryptographicHash::Sha1).toHex () } };

		QString tmpString;
		auto cvt = [] (const QStringList& list) { return list.join ("; "); };

		const auto issuer = new QTreeWidgetItem { item, { QObject::tr ("Issuer info") } };
		auto mkIssuerItem = [&cvt, &cer, issuer] (const QString& name,
				QSslCertificate::SubjectInfo field)
		{
			const auto& value = cvt (cer.issuerInfo (field));
			if (!value.isEmpty ())
				new QTreeWidgetItem { issuer, { name, value } };
		};

		mkIssuerItem (QObject::tr ("Organization:"), QSslCertificate::Organization);
		mkIssuerItem (QObject::tr ("Common name:"), QSslCertificate::CommonName);
		mkIssuerItem (QObject::tr ("Locality:"), QSslCertificate::LocalityName);
		mkIssuerItem (QObject::tr ("Organizational unit name:"), QSslCertificate::OrganizationalUnitName);
		mkIssuerItem (QObject::tr ("Country name:"), QSslCertificate::CountryName);
		mkIssuerItem (QObject::tr ("State or province name:"), QSslCertificate::StateOrProvinceName);

		const auto subject = new QTreeWidgetItem { item, { QObject::tr ("Subject info") } };
		auto mkSubjectItem = [&cvt, &cer, subject] (const QString& name,
				QSslCertificate::SubjectInfo field)
		{
			const auto& value = cvt (cer.subjectInfo (field));
			if (!value.isEmpty ())
				new QTreeWidgetItem { subject, { name, value } };
		};

		mkSubjectItem (QObject::tr ("Organization:"), QSslCertificate::Organization);
		mkSubjectItem (QObject::tr ("Common name:"), QSslCertificate::CommonName);
		mkSubjectItem (QObject::tr ("Locality:"), QSslCertificate::LocalityName);
		mkSubjectItem (QObject::tr ("Organizational unit name:"), QSslCertificate::OrganizationalUnitName);
		mkSubjectItem (QObject::tr ("Country name:"), QSslCertificate::CountryName);
		mkSubjectItem (QObject::tr ("State or province name:"), QSslCertificate::StateOrProvinceName);

		return item;
	}
}
}
