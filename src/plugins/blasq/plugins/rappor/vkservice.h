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

#include <QObject>
#include <interfaces/blasq/iservice.h>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Blasq
{
namespace Rappor
{
	class VkAccount;

	class VkService : public QObject
					, public IService
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blasq::IService)

		const ICoreProxy_ptr Proxy_;

		QList<VkAccount*> Accounts_;
	public:
		VkService (ICoreProxy_ptr);

		QObject* GetQObject ();

		QString GetServiceName () const;
		QIcon GetServiceIcon () const;

		QList<IAccount*> GetRegisteredAccounts () const;
		QList<QWidget*> GetAccountRegistrationWidgets () const;
		void RegisterAccount (const QString&, const QList<QWidget*>&);
		void RemoveAccount (IAccount*);
	private:
		void AddAccount (VkAccount*);
	private slots:
		void saveAccount (VkAccount*);
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);
	};
}
}
}
