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

#include "tagsmanager.h"
#include <stdexcept>
#include <algorithm>
#include <QStringList>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <util/tags/tagscompleter.h>

using namespace LC;

TagsManager::TagsManager ()
{
	ReadSettings ();
	GetID (tr ("untagged"));
	Util::TagsCompleter::SetModel (GetModel ());
}

TagsManager& TagsManager::Instance ()
{
	static TagsManager tm;
	return tm;
}

int TagsManager::columnCount (const QModelIndex&) const
{
	return 1;
}

QVariant TagsManager::data (const QModelIndex& index, int role) const
{
	if (!index.isValid () ||
			role != Qt::DisplayRole)
		return QVariant ();

	TagsDictionary_t::const_iterator pos = Tags_.begin ();
	std::advance (pos, index.row ());
	return *pos;
}

QModelIndex TagsManager::index (int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex TagsManager::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int TagsManager::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : Tags_.size ();
}

ITagsManager::tag_id TagsManager::GetID (const QString& tag)
{
	const auto& keys = Tags_.keys (tag);
	if (keys.isEmpty ())
		return InsertTag (tag);
	else if (keys.size () > 1)
		throw std::runtime_error (qPrintable (QString ("More than one key for %1").arg (tag)));
	else
		return keys.at (0).toString ();
}

QString TagsManager::GetTag (ITagsManager::tag_id id) const
{
	return Tags_ [id];
}

QStringList TagsManager::GetAllTags () const
{
	return Tags_.values ();
}

QStringList TagsManager::Split (const QString& string) const
{
	return Util::Map (string.split (";", QString::SkipEmptyParts),
			[] (QString& s) { return std::move (s).trimmed (); });
}

QList<ITagsManager::tag_id> TagsManager::SplitToIDs (const QString& string)
{
	return GetIDs (Split (string));
}

QString TagsManager::Join (const QStringList& tags) const
{
	return tags.join ("; ");
}

QString TagsManager::JoinIDs (const QStringList& tagIDs) const
{
	const auto& hr = Util::Map (tagIDs,
			[this] (const QString& id) { return GetTag (id); });
	return Join (hr);
}

ITagsManager::tag_id TagsManager::InsertTag (const QString& tag)
{
	const auto& uuid = QUuid::createUuid ();

	auto updated = Tags_;
	auto pos = updated.insert (uuid, tag);
	const auto dist = std::distance (updated.begin (), pos);

	beginInsertRows ({}, dist, dist);
	Tags_ = std::move (updated);
	endInsertRows ();

	WriteSettings ();

	emit tagsUpdated (GetAllTags ());

	return uuid.toString ();
}

void TagsManager::RemoveTag (const QModelIndex& index)
{
	if (!index.isValid ())
		return;

	beginRemoveRows (QModelIndex (), index.row (), index.row ());
	Tags_.erase (Tags_.begin () + index.row ());
	endRemoveRows ();
	WriteSettings ();
	emit tagsUpdated (GetAllTags ());
}

void TagsManager::SetTag (const QModelIndex& index, const QString& newTag)
{
	if (!index.isValid ())
		return;

	auto pos = Tags_.begin () + index.row ();
	*pos = newTag;

	emit dataChanged (index, index);

	WriteSettings ();

	emit tagsUpdated (GetAllTags ());
}

QAbstractItemModel* TagsManager::GetModel ()
{
	return this;
}

QObject* TagsManager::GetQObject ()
{
	return this;
}

void TagsManager::ReadSettings ()
{
	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName ());
	settings.beginGroup ("Tags");
	Tags_ = settings.value ("Dict").value<TagsDictionary_t> ();
	if (!Tags_.isEmpty ())
	{
		beginInsertRows (QModelIndex (), 0, Tags_.size () - 1);
		endInsertRows ();
	}
	settings.endGroup ();
}

void TagsManager::WriteSettings () const
{
	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName ());
	settings.beginGroup ("Tags");
	settings.setValue ("Dict", QVariant::fromValue<TagsDictionary_t> (Tags_));
	settings.endGroup ();
}

