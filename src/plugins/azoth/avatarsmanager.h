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

#include <functional>
#include <QObject>
#include <QHash>
#include <util/sll/util.h>
#include "interfaces/azoth/ihaveavatars.h"
#include "interfaces/azoth/iproxyobject.h"

template<typename>
class QFuture;

namespace LeechCraft
{
namespace Azoth
{
	class AvatarsStorage;
	class IAccount;

	class AvatarsManager : public QObject
						 , public IAvatarsManager
	{
		Q_OBJECT

		AvatarsStorage * const Storage_;

		QHash<QObject*, QHash<IHaveAvatars::Size, QFuture<QImage>>> PendingRequests_;
	public:
		using AvatarHandler_f = std::function<void (QImage)>;
	private:
		uint64_t SubscriptionID_ = 0;
		QHash<QObject*, QHash<IHaveAvatars::Size, QHash<uint64_t, AvatarHandler_f>>> Subscriptions_;

		QHash<QObject*, const IAccount*> SelfInfo2Account_;
	public:
		explicit AvatarsManager (QObject* = nullptr);

		QFuture<QImage> GetAvatar (QObject*, IHaveAvatars::Size) override;
		QFuture<std::optional<QByteArray>> GetStoredAvatarData (const QString&, IHaveAvatars::Size) override;

		bool HasAvatar (QObject*) const;

		Util::DefaultScopeGuard Subscribe (QObject*, IHaveAvatars::Size, const AvatarHandler_f&);
	private:
		void HandleSubscriptions (QObject*);
	public slots:
		void handleAccount (QObject*);
	private slots:
		void handleEntries (const QList<QObject*>&);
		void invalidateAvatar (QObject*);

		void handleCacheSizeChanged ();
	signals:
		void avatarInvalidated (QObject*);
		void accountAvatarInvalidated (const IAccount*);
	};
}
}
