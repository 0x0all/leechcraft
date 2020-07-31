/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "workerobject.h"
#include <QUrl>
#include <QFile>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElementCollection>
#include <QtConcurrentRun>
#include <QTextCodec>
#include <QTimer>
#include <QApplication>
#include <QtDebug>
#include <interfaces/iscriptloader.h>
#include <util/sys/paths.h>
#include <util/sll/util.h>
#include <util/sll/prelude.h>
#include <util/threads/futures.h>
#include <interfaces/aggregator/iproxyobject.h>
#include <interfaces/aggregator/item.h>
#include <interfaces/aggregator/channel.h>

uint qHash (IScript_ptr script)
{
	return qHash (script.get ());
}

namespace LC
{
namespace Aggregator
{
namespace BodyFetch
{
	WorkerObject::WorkerObject (IProxyObject *proxy, QObject *parent)
	: QObject { parent }
	, AggregatorProxy_ { proxy }
	, StorageDir_ { Util::CreateIfNotExists ("aggregator/bodyfetcher/storage") }
	{
		QTimer *timer = new QTimer { this };
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (clearCaches ()));
		timer->start (10000);
	}

	void WorkerObject::SetLoaderInstance (const IScriptLoaderInstance_ptr& inst)
	{
		Inst_ = inst;
	}

	bool WorkerObject::IsOk () const
	{
		return Inst_.get ();
	}

	void WorkerObject::AppendItem (const Item& item)
	{
		Items_ << item;

		QTimer::singleShot (500,
				this,
				SLOT (process ()));
	}

	void WorkerObject::ProcessItems (const QList<Item>& items)
	{
		if (!Inst_)
		{
			qWarning () << Q_FUNC_INFO
					<< "null instance loader, aborting";
			return;
		}

		if (EnumeratedCache_.isEmpty ())
			EnumeratedCache_ = Inst_->EnumerateScripts ();

		QHash<QString, IScript_ptr> channel2script;

		for (const auto& item : items)
		{
			const auto& channel = AggregatorProxy_->GetChannel (item.ChannelID_);
			if (channel.ChannelID_ == IDNotFound)
				continue;

			const auto& channelLinkStr = channel.Link_;

			auto script = channel2script.value (channelLinkStr);
			if (!script)
			{
				script = GetScriptForChannel (channelLinkStr);
				if (!script)
					continue;
				channel2script [channelLinkStr] = script;
			}

			const QVariantList args
			{
				item.Link_,
				item.CommentsPageLink_,
				item.Description_
			};
			auto fetchStr = script->InvokeMethod ("GetFullURL", args).toString ();
			if (fetchStr.isEmpty ())
				fetchStr = item.Link_;

			qDebug () << Q_FUNC_INFO << fetchStr << "using" << ChannelLink2ScriptID_ [channelLinkStr];

			const auto& url = QUrl::fromEncoded (fetchStr.toUtf8 ());
			URL2Script_ [url] = script;
			URL2ItemID_ [url] = item.ItemID_;
			emit downloadRequested (url);
		}
	}

	IScript_ptr WorkerObject::GetScriptForChannel (const QString& channel)
	{
		if (CachedScripts_.contains (channel))
			return CachedScripts_ [channel];

		IScript_ptr script;
		if (ChannelLink2ScriptID_.contains (channel))
		{
			script = Inst_->LoadScript (ChannelLink2ScriptID_ [channel]);
			if (!script ||
					!script->InvokeMethod ("CanHandle", { channel }).toBool ())
			{
				ChannelLink2ScriptID_.remove (channel);
				script.reset ();
			}
		}

		if (!ChannelLink2ScriptID_.contains (channel))
		{
			const auto& scriptId = FindScriptForChannel (channel);
			if (scriptId.isEmpty ())
			{
				CachedScripts_ [channel] = {};
				return {};
			}

			ChannelLink2ScriptID_ [channel] = scriptId;
		}

		if (ChannelLink2ScriptID_ [channel].isEmpty ())
		{
			ChannelLink2ScriptID_.remove (channel);
			CachedScripts_ [channel] = {};
			return {};
		}

		if (!script)
			script = Inst_->LoadScript (ChannelLink2ScriptID_ [channel]);

		CachedScripts_ [channel] = script;

		return script;
	}

	QString WorkerObject::FindScriptForChannel (const QString& link)
	{
		for (const auto& id : EnumeratedCache_)
		{
			const auto& script = Inst_->LoadScript (id);
			if (script->InvokeMethod ("CanHandle", { link }).toBool ())
				return id;
		}

		return QString ();
	}

	namespace
	{
		QStringList GetReplacements (IScript_ptr script, const QString& method)
		{
			const auto& var = script->InvokeMethod (method, {});
			auto result = Util::Map (var.toList (), &QVariant::toString);
			result.removeAll ({});
			result.removeDuplicates ();
			return result;
		}

		template<typename Func>
		QString ParseWithSelectors (QWebFrame *frame,
				const QStringList& selectors,
				int amount,
				Func func)
		{
			QString result;

			for (const auto& sel : selectors)
			{
				const auto& col = frame->findAllElements (sel);
				for (int i = 0, size = std::min (amount, col.count ()); i < size; ++i)
					result += func (col.at (i)).simplified ();
			}

			return result;
		}
	}

	QString WorkerObject::Parse (const QString& contents, IScript_ptr script)
	{
		const QStringList& firstTagOut = GetReplacements (script, "KeepFirstTag");
		const QStringList& allTagsOut = GetReplacements (script, "KeepAllTags");
		const QStringList& firstTagIn = GetReplacements (script, "KeepFirstTagInnerXml");

		if (firstTagOut.isEmpty () &&
				allTagsOut.isEmpty () &&
				firstTagIn.isEmpty ())
			return script->InvokeMethod ("Strip", { contents }).toString ();

		QWebPage page;
		page.settings ()->setAttribute (QWebSettings::DeveloperExtrasEnabled, false);
		page.settings ()->setAttribute (QWebSettings::JavascriptEnabled, false);
		page.settings ()->setAttribute (QWebSettings::AutoLoadImages, false);
		page.settings ()->setAttribute (QWebSettings::PluginsEnabled, false);
		page.mainFrame ()->setHtml (contents);

		QString result;
		result += ParseWithSelectors (page.mainFrame (),
				firstTagOut, 1, [] (const QWebElement& e) { return e.toOuterXml (); });
		result += ParseWithSelectors (page.mainFrame (),
				allTagsOut, 1000, [] (const QWebElement& e) { return e.toOuterXml (); });
		result += ParseWithSelectors (page.mainFrame (),
				firstTagIn, 1, [] (const QWebElement& e) { return e.toInnerXml (); });

		result.remove ("</br>");

		return result;
	}

	void WorkerObject::WriteFile (const QString& contents, quint64 itemId) const
	{
		QDir dir = StorageDir_;
		dir.cd (QString::number (itemId % 10));

		QFile file (dir.filePath (QString ("%1.html").arg (itemId)));
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}

		file.write (contents.toUtf8 ());
	}

	QString WorkerObject::Recode (const QByteArray& rawContents) const
	{
		const QByteArray stupidCharset ("meta charset=");
		const int stupidPos = rawContents.indexOf (stupidCharset);

		if (stupidPos >= 0)
		{
			const int begin = stupidPos + stupidCharset.size ();
			const char sep = rawContents.at (begin);
			if (sep == '\'' || sep == '"')
			{
				const int end = rawContents.indexOf (sep, begin + 1);

				const auto& enca = rawContents.mid (begin + 1, end - begin - 1);
				qDebug () << "detected encoding" << enca;
				if (const auto codec = QTextCodec::codecForName (enca))
					return codec->toUnicode (rawContents);
				else
					qWarning () << Q_FUNC_INFO
							<< "unable to get codec for"
							<< enca;
			}
		}

		const auto codec = QTextCodec::codecForHtml (rawContents, 0);
		return codec ?
				codec->toUnicode (rawContents) :
				QString::fromUtf8 (rawContents);
	}

	void WorkerObject::ScheduleRechecking ()
	{
		if (RecheckScheduled_)
			return;

		QTimer::singleShot (1000,
				this,
				SLOT (recheckFinished ()));

		RecheckScheduled_ = true;
	}

	void WorkerObject::handleDownloadFinished (QUrl url, QString filename)
	{
		if (IsProcessing_)
		{
			FetchedQueue_.append ({ url, filename });
			ScheduleRechecking ();
			return;
		}

		IsProcessing_ = true;
		const auto pg = Util::MakeScopeGuard ([this] { IsProcessing_ = false; });

		const auto& script = URL2Script_.take (url);
		if (!script)
		{
			qWarning () << Q_FUNC_INFO
					<< "null script for"
					<< url;
			return;
		}

		const auto file = std::make_shared<QFile> (filename);
		if (!file->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< filename
					<< file->errorString ();
			file->remove ();
			return;
		}

		Util::Sequence (this,
					QtConcurrent::run ([this, file]
					{
						const auto& contents = file->readAll ();
						file->close ();
						file->remove ();
						return Recode (contents);
					})) >>
				[this, url, script] (const QString& contents)
				{
					const auto& result = Parse (contents, script);
					if (result.isEmpty ())
					{
						qWarning () << Q_FUNC_INFO
								<< "empty result for"
								<< url;
						return;
					}

					const quint64 id = URL2ItemID_.take (url);
					WriteFile (result, id);
					emit newBodyFetched (id);
					qDebug () << Q_FUNC_INFO << "done!" << url;
				};
	}

	void WorkerObject::recheckFinished ()
	{
		RecheckScheduled_ = false;

		if (FetchedQueue_.isEmpty ())
			return;

		if (IsProcessing_)
			ScheduleRechecking ();

		const auto& item = FetchedQueue_.takeFirst ();
		handleDownloadFinished (item.first, item.second);
	}

	void WorkerObject::process ()
	{
		if (Items_.isEmpty ())
			return;

		if (!IsProcessing_)
			ProcessItems ({ Items_.takeFirst () });

		if (!Items_.isEmpty ())
			QTimer::singleShot (400,
					this,
					SLOT (process ()));
	}

	void WorkerObject::clearCaches ()
	{
		if (IsProcessing_)
			return;

		EnumeratedCache_.clear ();
		CachedScripts_.clear ();
	}
}
}
}
