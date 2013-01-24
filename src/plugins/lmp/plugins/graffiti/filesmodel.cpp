/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "filesmodel.h"
#include <QtDebug>
#include <QColor>

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	FilesModel::File::File (const QFileInfo& fi)
	: Path_ (fi.absoluteFilePath ())
	, Name_ (fi.fileName ())
	, IsChanged_ (false)
	{
	}

	FilesModel::FilesModel (QObject *parent)
	: QAbstractItemModel (parent)
	, Headers_ ({ tr ("Track"), tr ("Album"), tr ("Artist"), tr ("File name") })
	{
	}

	QModelIndex FilesModel::index (int row, int column, const QModelIndex& parent) const
	{
		return parent.isValid () ? QModelIndex () : createIndex (row, column);
	}

	QModelIndex FilesModel::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int FilesModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : Files_.size ();
	}

	int FilesModel::columnCount (const QModelIndex&) const
	{
		return Headers_.size ();
	}

	QVariant FilesModel::headerData (int section, Qt::Orientation orientation, int role) const
	{
		if (orientation != Qt::Horizontal)
			return QVariant ();

		if (role != Qt::DisplayRole)
			return QVariant ();

		return Headers_.at (section);
	}

	QVariant FilesModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return QVariant ();

		switch (role)
		{
		case Qt::DisplayRole:
		{
			const auto& file = Files_.at (index.row ());
			switch (index.column ())
			{
			case Columns::Filename:
				return file.Name_;
			case Columns::Artist:
				return file.Info_.Artist_;
			case Columns::Album:
				return file.Info_.Album_;
			case Columns::Title:
				return file.Info_.Title_;
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown column"
					<< index.column ();
			return QVariant ();
		}
		case Qt::ForegroundRole:
			return Files_.at (index.row ()).IsChanged_ ?
					QVariant::fromValue (QColor (Qt::red)) :
					QVariant ();
		case Roles::MediaInfoRole:
			return QVariant::fromValue (Files_.at (index.row ()).Info_);
		case Roles::OrigMediaInfo:
			return QVariant::fromValue (Files_.at (index.row ()).OrigInfo_);
		default:
			return QVariant ();
		}
	}

	void FilesModel::AddFiles (const QList<QFileInfo>& files)
	{
		if (files.isEmpty ())
			return;

		beginInsertRows (QModelIndex (), Files_.size (), files.size () + Files_.size ());
		std::copy (files.begin (), files.end (), std::back_inserter (Files_));
		endInsertRows ();
	}

	void FilesModel::SetInfos (const QList<MediaInfo>& infos)
	{
		for (const auto& info : infos)
		{
			const auto pos = FindFile (info.LocalPath_);
			if (pos == Files_.end ())
				continue;

			pos->Info_ = info;
			pos->OrigInfo_ = info;
			pos->IsChanged_ = false;

			const auto row = std::distance (Files_.begin (), pos);
			emit dataChanged (index (row, 0), index (row, Columns::MaxColumn - 1));
		}
	}

	void FilesModel::UpdateInfo (const QModelIndex& idx, const MediaInfo& info)
	{
		if (!idx.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index"
					<< idx;
			return;
		}

		const int row = idx.row ();
		auto& file = Files_ [row];

		if (file.Info_ == info)
			return;

		file.Info_ = info;
		file.IsChanged_ = info != file.OrigInfo_;
		emit dataChanged (index (row, 0), index (row, Columns::MaxColumn - 1));
	}

	void FilesModel::Clear ()
	{
		if (Files_.isEmpty ())
			return;

		beginRemoveRows (QModelIndex (), 0, Files_.size ());
		Files_.clear ();
		endRemoveRows ();
	}

	QList<QPair<MediaInfo, MediaInfo>> FilesModel::GetModified () const
	{
		QList<QPair<MediaInfo, MediaInfo>> result;
		for (const auto& file : Files_)
			if (file.Info_ != file.OrigInfo_)
				result.push_back ({ file.Info_, file.OrigInfo_ });
		return result;
	}

	QList<FilesModel::File>::iterator FilesModel::FindFile (const QString& path)
	{
		return std::find_if (Files_.begin (), Files_.end (),
				[&path] (const File& file) { return file.Path_ == path; });
	}
}
}
}
