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

#include "localcollectionmodel.h"
#include <QUrl>
#include <QMimeData>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	LocalCollectionModel::LocalCollectionModel (QObject *parent)
	: QStandardItemModel { parent }
	{
		setSupportedDragActions (Qt::CopyAction);
	}

	QStringList LocalCollectionModel::mimeTypes () const
	{
		return { "text/uri-list" };
	}

	namespace
	{
		QStringList CollectPaths (const QModelIndex& index, const QAbstractItemModel *model)
		{
			const auto type = index.data (LocalCollectionModel::Role::Node).toInt ();
			if (type == LocalCollectionModel::NodeType::Track)
				return { index.data (LocalCollectionModel::Role::TrackPath).toString () };

			QStringList paths;
			for (int i = 0; i < model->rowCount (index); ++i)
				paths += CollectPaths (model->index (i, 0, index), model);
			return paths;
		}
	}

	QMimeData* LocalCollectionModel::mimeData (const QModelIndexList& indexes) const
	{
		QList<QUrl> urls;
		for (const auto& index : indexes)
		{
			const auto& paths = CollectPaths (index, this);
			std::transform (paths.begin (), paths.end (), std::back_inserter (urls),
					[] (const QString& path) { return QUrl::fromLocalFile (path); });
		}
		if (urls.isEmpty ())
			return nullptr;

		auto result = new QMimeData;
		result->setUrls (urls);
		return result;
	}

	QList<QUrl> LocalCollectionModel::ToSourceUrls (const QList<QModelIndex>& indexes) const
	{
		const auto& paths = std::accumulate (indexes.begin (), indexes.end (), QStringList {},
				[this] (const QStringList& paths, decltype (indexes.front ()) item)
					{ return paths + CollectPaths (item, this); });

		QList<QUrl> result;
		result.reserve (paths.size ());
		for (const auto& path : paths)
			result << QUrl::fromLocalFile (path);
		return result;
	}

	void LocalCollectionModel::FinalizeInit ()
	{
		ArtistIcon_ = Core::Instance ().GetProxy ()->
				GetIconThemeManager ()->GetIcon ("view-media-artist");
	}

	namespace
	{
		template<typename T, typename U, typename Init, typename Parent>
		QStandardItem* GetItem (T& c, U idx, Init f, Parent parent)
		{
			auto item = c [idx];
			if (item)
				return item;

			item = new QStandardItem ();
			item->setEditable (false);
			f (item);
			parent->appendRow (item);
			c [idx] = item;
			return item;
		}
	}

	void LocalCollectionModel::AddArtists (const Collection::Artists_t& artists)
	{
		for (const auto& artist : artists)
		{
			auto artistItem = GetItem (Artist2Item_,
					artist.ID_,
					[this, &artist] (QStandardItem *item)
					{
						item->setIcon (ArtistIcon_);
						item->setText (artist.Name_);
						item->setData (artist.Name_, Role::ArtistName);
						item->setData (NodeType::Artist, Role::Node);
					},
					this);
			for (auto album : artist.Albums_)
			{
				auto albumItem = GetItem (Album2Item_,
						album->ID_,
						[album, artist] (QStandardItem *item)
						{
							item->setText (QString::fromUtf8 ("%1 — %2")
									.arg (album->Year_)
									.arg (album->Name_));
							item->setData (album->Year_, Role::AlbumYear);
							item->setData (album->Name_, Role::AlbumName);
							item->setData (artist.Name_, Role::ArtistName);
							item->setData (NodeType::Album, Role::Node);
							if (!album->CoverPath_.isEmpty ())
								item->setData (album->CoverPath_, Role::AlbumArt);
						},
						artistItem);

				for (const auto& track : album->Tracks_)
				{
					const QString& name = QString::fromUtf8 ("%1 — %2")
							.arg (track.Number_)
							.arg (track.Name_);
					auto item = new QStandardItem (name);
					item->setEditable (false);
					item->setData (album->Year_, Role::AlbumYear);
					item->setData (album->Name_, Role::AlbumName);
					item->setData (artist.Name_, Role::ArtistName);
					item->setData (track.Number_, Role::TrackNumber);
					item->setData (track.Name_, Role::TrackTitle);
					item->setData (track.FilePath_, Role::TrackPath);
					item->setData (track.Genres_, Role::TrackGenres);
					item->setData (track.Length_, Role::TrackLength);
					item->setData (NodeType::Track, Role::Node);
					albumItem->appendRow (item);

					Track2Item_ [track.ID_] = item;
				}
			}
		}
	}

	void LocalCollectionModel::Clear ()
	{
		clear ();

		Artist2Item_.clear ();
		Album2Item_.clear ();
		Track2Item_.clear ();
	}

	void LocalCollectionModel::RemoveTrack (int id)
	{
		auto item = Track2Item_.take (id);
		item->parent ()->removeRow (item->row ());
	}

	void LocalCollectionModel::RemoveAlbum (int id)
	{
		auto item = Album2Item_.take (id);
		item->parent ()->removeRow (item->row ());
	}

	QVariant LocalCollectionModel::GetTrackData (int trackId, LocalCollectionModel::Role role) const
	{
		const auto item = Track2Item_.value (trackId);
		return item ? item->data (role) : QVariant ();
	}

	void LocalCollectionModel::RemoveArtist (int id)
	{
		removeRow (Artist2Item_.take (id)->row ());
	}

	void LocalCollectionModel::SetAlbumArt (int id, const QString& path)
	{
		if (Album2Item_.contains (id))
			Album2Item_ [id]->setData (path, Role::AlbumArt);
	}
}
}
