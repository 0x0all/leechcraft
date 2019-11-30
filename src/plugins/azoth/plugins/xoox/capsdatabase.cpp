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

#include "capsdatabase.h"
#include <QDataStream>
#include <QFile>
#include <QTimer>
#include <util/util.h>
#include <util/sys/paths.h>
#include "capsstorageondisk.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	CapsDatabase::CapsDatabase (const ILoadProgressReporter_ptr& lpr, QObject *parent)
	: QObject { parent }
	, Storage_ { new CapsStorageOnDisk { lpr, this } }
	{
	}

	bool CapsDatabase::Contains (const QByteArray& hash) const
	{
		if (Ver2Features_.contains (hash) && Ver2Identities_.contains (hash))
			return true;

		return Preload (hash);
	}

	QStringList CapsDatabase::Get (const QByteArray& hash) const
	{
		if (!Ver2Features_.contains (hash))
			Preload (hash);

		return Ver2Features_ [hash];
	}

	void CapsDatabase::Set (const QByteArray& hash, const QStringList& features)
	{
		Ver2Features_ [hash] = features;
		Storage_->AddFeatures (hash, features);
	}

	QList<QXmppDiscoveryIq::Identity> CapsDatabase::GetIdentities (const QByteArray& hash) const
	{
		if (!Ver2Identities_.contains (hash))
			Preload (hash);

		return Ver2Identities_ [hash];
	}

	void CapsDatabase::SetIdentities (const QByteArray& hash,
			const QList<QXmppDiscoveryIq::Identity>& ids)
	{
		Ver2Identities_ [hash] = ids;
		Storage_->AddIdentities (hash, ids);
	}

	bool CapsDatabase::Preload (const QByteArray& hash) const
	{
		const auto& features = Storage_->GetFeatures (hash);
		const auto& identities = Storage_->GetIdentities (hash);
		if (!features || !identities)
			return false;

		Ver2Features_ [hash] = *features;
		Ver2Identities_ [hash] = *identities;
		return true;
	}
}
}
}
