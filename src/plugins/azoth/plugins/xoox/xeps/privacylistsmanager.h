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
#include <QXmppClientExtension.h>
#include <util/sll/eithercont.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class PrivacyListItem
	{
	public:
		enum class Type
		{
			None,
			Jid,
			Group,
			Subscription
		};

		enum class Action
		{
			Allow,
			Deny
		};

		enum StanzaType
		{
			STNone = 0x00,
			STMessage = 0x01,
			STPresenceIn = 0x02,
			STPresenceOut = 0x04,
			STIq = 0x08,
			STAll = 0x0f
		};

		Q_DECLARE_FLAGS (StanzaTypes, StanzaType)
	private:
		QString Value_;
		Type Type_;
		Action Action_;
		StanzaTypes Stanzas_;
	public:
		PrivacyListItem (const QString& = {}, Type = Type::None, Action = Action::Deny);

		void Parse (const QDomElement&);
		QXmppElement ToXML () const;

		Type GetType () const;
		void SetType (Type);

		Action GetAction () const;
		void SetAction (Action);

		QString GetValue () const;
		void SetValue (const QString&);

		StanzaTypes GetStanzaTypes () const;
		void SetStanzaTypes (StanzaTypes);
	};

	bool operator== (const PrivacyListItem&, const PrivacyListItem&);

	class PrivacyList
	{
		QString Name_;
		QList<PrivacyListItem> Items_;
	public:
		PrivacyList (const QString& = QString ());

		void Parse (const QDomElement&);
		QXmppElement ToXML () const;

		QString GetName () const;
		void SetName (const QString&);

		const QList<PrivacyListItem>& GetItems () const;
		void SetItems (const QList<PrivacyListItem>&);
	};

	class ClientConnection;

	class PrivacyListsManager : public QXmppClientExtension
	{
		Q_OBJECT

		ClientConnection * const Conn_;

		enum class QueryType
		{
			QueryLists,
			GetList
		};
		QMap<QString, QueryType> ID2Type_;
	public:
		using QueryListsCont_f = Util::EitherCont<void (QXmppIq), void (QStringList, QString, QString)>;
		using QueryListCont_f = Util::EitherCont<void (QXmppIq), void (PrivacyList)>;
	private:
		QMap<QString, QueryListsCont_f> QueryLists2Handler_;
		QMap<QString, QueryListCont_f> QueryList2Handler_;

		QString CurrentName_;
		PrivacyList CurrentList_;
	public:
		PrivacyListsManager (ClientConnection*);

		enum class ListType
		{
			Active,
			Default
		};

		bool IsSupported () const;

		void QueryLists ();
		void QueryLists (const QueryListsCont_f&);

		void QueryList (const QString&);
		void QueryList (const QString&, const QueryListCont_f&);

		void ActivateList (const QString&, ListType = ListType::Active);
		void SetList (const PrivacyList&);

		const PrivacyList& GetCurrentList () const;

		QStringList discoveryFeatures () const;
		bool handleStanza (const QDomElement&);
	private:
		void HandleListQueryError (const QXmppIq&);
		void HandleListQueryResult (const QDomElement&);
		void HandleList (const QDomElement&);
	signals:
		void gotLists (const QStringList& lists, const QString& active, const QString& def);
		void gotList (const PrivacyList&);
		void currentListFetched (const PrivacyList&);

		void listError (const QString&);
	};
}
}
}
