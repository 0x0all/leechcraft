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

#include "aggregatortab.h"
#include <QKeyEvent>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <util/models/flattofoldersproxymodel.h>
#include <util/sll/prelude.h>
#include <util/tags/tagscompleter.h>
#include <util/util.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "uistatepersist.h"
#include "channelsmodel.h"
#include "channelsfiltermodel.h"

namespace LeechCraft
{
namespace Aggregator
{
	AggregatorTab::AggregatorTab (const InitParams& params, QObject *plugin)
	: TabClass_ { params.TabClass_ }
	, ParentPlugin_ { plugin }
	, ChannelActions_ { params.ChannelActions_ }
	, FlatToFolders_ { std::make_shared<Util::FlatToFoldersProxyModel> () }
	, ChannelsFilterModel_ { new ChannelsFilterModel { this } }
	{
		ChannelsFilterModel_->setSourceModel (params.ChannelsModel_);
		ChannelsFilterModel_->setFilterKeyColumn (0);

		Ui_.setupUi (this);
		Ui_.ItemsWidget_->SetAppWideActions (params.AppWideActions_);
		Ui_.ItemsWidget_->SetChannelActions (*ChannelActions_);
		Ui_.ItemsWidget_->RegisterShortcuts (params.ShortcutMgr_);

		Ui_.ItemsWidget_->SetChannelsFilter (ChannelsFilterModel_);

		connect (Ui_.ItemsWidget_,
				&ItemsWidget::movedToChannel,
				this,
				&AggregatorTab::handleItemsMovedToChannel);

		Ui_.MergeItems_->setChecked (XmlSettingsManager::Instance ()->Property ("MergeItems", false).toBool ());

		Ui_.Feeds_->addAction (ChannelActions_->ActionMarkChannelAsRead_);
		Ui_.Feeds_->addAction (ChannelActions_->ActionMarkChannelAsUnread_);
		Ui_.Feeds_->addAction (Util::CreateSeparator (Ui_.Feeds_));
		Ui_.Feeds_->addAction (ChannelActions_->ActionRemoveFeed_);
		Ui_.Feeds_->addAction (ChannelActions_->ActionUpdateSelectedFeed_);
		Ui_.Feeds_->addAction (ChannelActions_->ActionRenameFeed_);
		Ui_.Feeds_->addAction (Util::CreateSeparator (Ui_.Feeds_));
		Ui_.Feeds_->addAction (ChannelActions_->ActionRemoveChannel_);
		Ui_.Feeds_->addAction (Util::CreateSeparator (Ui_.Feeds_));
		Ui_.Feeds_->addAction (ChannelActions_->ActionChannelSettings_);
		Ui_.Feeds_->addAction (Util::CreateSeparator (Ui_.Feeds_));
		Ui_.Feeds_->addAction (params.AppWideActions_.ActionAddFeed_);

		connect (Ui_.Feeds_,
				&QWidget::customContextMenuRequested,
				this,
				&AggregatorTab::handleFeedsContextMenuRequested);

		const auto fm = fontMetrics ();
		int dateTimeSize = fm.width (QDateTime::currentDateTime ().toString (Qt::SystemLocaleShortDate) + "__");
		const auto channelsHeader = Ui_.Feeds_->header ();
		channelsHeader->resizeSection (0, fm.width ("Average channel name"));
		channelsHeader->resizeSection (1, fm.width ("_9999_"));
		channelsHeader->resizeSection (2, dateTimeSize);

		connect (Ui_.TagsLine_,
				&QLineEdit::textChanged,
				ChannelsFilterModel_,
				&QSortFilterProxyModel::setFilterFixedString);

		new Util::TagsCompleter (Ui_.TagsLine_);
		Ui_.TagsLine_->AddSelector ();

		Ui_.MainSplitter_->setStretchFactor (0, 5);
		Ui_.MainSplitter_->setStretchFactor (1, 9);

		connect (FlatToFolders_.get (),
				&QAbstractItemModel::rowsInserted,
				Ui_.Feeds_,
				&QTreeView::expand);

		LoadColumnWidth (Ui_.Feeds_, "feeds");
		Ui_.ItemsWidget_->ConstructBrowser ();
		Ui_.ItemsWidget_->LoadUIState ();

		UiStateGuard_ = Util::MakeScopeGuard ([this]
				{
					SaveColumnWidth (Ui_.Feeds_, "feeds");
					Ui_.ItemsWidget_->SaveUIState ();
				});

		FlatToFolders_->SetTagsManager (Core::Instance ().GetProxy ()->GetTagsManager ());
		handleGroupChannels ();
		XmlSettingsManager::Instance ()->RegisterObject ("GroupChannelsByTags", this, "handleGroupChannels");

		currentChannelChanged ();
	}

	QToolBar* AggregatorTab::GetToolBar () const
	{
		return Ui_.ItemsWidget_->GetToolBar ();
	}

	TabClassInfo AggregatorTab::GetTabClassInfo () const
	{
		return TabClass_;
	}

	QObject* AggregatorTab::ParentMultiTabs ()
	{
		return ParentPlugin_;
	}

	void AggregatorTab::Remove ()
	{
		emit removeTabRequested ();
	}

	QByteArray AggregatorTab::GetTabRecoverData () const
	{
		return "aggregatortab";
	}

	QIcon AggregatorTab::GetTabRecoverIcon () const
	{
		return TabClass_.Icon_;
	}

	QString AggregatorTab::GetTabRecoverName () const
	{
		return TabClass_.VisibleName_;
	}

	QModelIndex AggregatorTab::GetRelevantIndex () const
	{
		auto index = Ui_.Feeds_->selectionModel ()->currentIndex ();
		if (FlatToFolders_->GetSourceModel ())
			index = FlatToFolders_->MapToSource (index);
		return ChannelsFilterModel_->mapToSource (index);
	}

	QList<QModelIndex> AggregatorTab::GetRelevantIndexes () const
	{
		auto rawList = Util::Map (Ui_.Feeds_->selectionModel ()->selectedRows (),
				[this] (QModelIndex index)
				{
					if (FlatToFolders_->GetSourceModel ())
						index = FlatToFolders_->MapToSource (index);
					return ChannelsFilterModel_->mapToSource (index);
				});
		rawList.removeAll ({});
		return rawList;
	}

	void AggregatorTab::keyPressEvent (QKeyEvent *e)
	{
		if (e->modifiers () & Qt::ControlModifier)
		{
			const auto channelSM = Ui_.Feeds_->selectionModel ();
			const auto& currentChannel = channelSM->currentIndex ();
			int numChannels = Ui_.Feeds_->model ()->rowCount (currentChannel.parent ());

			auto chanSF = QItemSelectionModel::Select |
					QItemSelectionModel::Clear |
					QItemSelectionModel::Rows;

			if (e->key () == Qt::Key_Less &&
					currentChannel.isValid ())
			{
				if (currentChannel.row () > 0)
				{
					const auto& next = currentChannel.sibling (currentChannel.row () - 1, currentChannel.column ());
					channelSM->select (next, chanSF);
					channelSM->setCurrentIndex (next, chanSF);
				}
				else
				{
					const auto& next = currentChannel.sibling (numChannels - 1, currentChannel.column ());
					channelSM->select (next, chanSF);
					channelSM->setCurrentIndex (next, chanSF);
				}
				return;
			}
			else if (e->key () == Qt::Key_Greater &&
					 currentChannel.isValid ())
			{
				if (currentChannel.row () < numChannels - 1)
				{
					const auto& next = currentChannel.sibling (currentChannel.row () + 1, currentChannel.column ());
					channelSM->select (next, chanSF);
					channelSM->setCurrentIndex (next, chanSF);
				}
				else
				{
					const auto& next = currentChannel.sibling (0, currentChannel.column ());
					channelSM->select (next, chanSF);
					channelSM->setCurrentIndex (next, chanSF);
				}
				return;
			}
			else if ((e->key () == Qt::Key_Greater ||
					  e->key () == Qt::Key_Less) &&
					 !currentChannel.isValid ())
			{
				const auto& next = Ui_.Feeds_->model ()->index (0, 0);
				channelSM->select (next, chanSF);
				channelSM->setCurrentIndex (next, chanSF);
			}
		}
		e->ignore ();
	}

	void AggregatorTab::handleItemsMovedToChannel (QModelIndex index)
	{
		if (index.column ())
			index = index.sibling (index.row (), 0);

		if (FlatToFolders_->GetSourceModel ())
		{
			const auto& sourceIdx = FlatToFolders_->MapFromSource (index).value (0);
			if (sourceIdx.isValid ())
				index = sourceIdx;
		}

		Ui_.Feeds_->blockSignals (true);
		Ui_.Feeds_->setCurrentIndex (index);
		Ui_.Feeds_->blockSignals (false);
	}

	void AggregatorTab::handleFeedsContextMenuRequested (const QPoint& pos)
	{
		bool enable = Ui_.Feeds_->indexAt (pos).isValid ();
		const QList<QAction*> toToggle
		{
			ChannelActions_->ActionMarkChannelAsRead_,
			ChannelActions_->ActionMarkChannelAsUnread_,
			ChannelActions_->ActionRemoveFeed_,
			ChannelActions_->ActionChannelSettings_,
			ChannelActions_->ActionUpdateSelectedFeed_
		};

		for (const auto act : toToggle)
			act->setEnabled (enable);

		QMenu *menu = new QMenu;
		menu->setAttribute (Qt::WA_DeleteOnClose, true);
		menu->addActions (Ui_.Feeds_->actions ());
		menu->exec (Ui_.Feeds_->viewport ()->mapToGlobal (pos));

		for (const auto act : toToggle)
			act->setEnabled (true);
	}

	void AggregatorTab::currentChannelChanged ()
	{
		const auto& index = Ui_.Feeds_->selectionModel ()->currentIndex ();
		const auto& mapped = FlatToFolders_->MapToSource (index);
		if (!mapped.isValid ())
		{
			const auto& tags = index.data (RoleTags).toStringList ();
			Ui_.ItemsWidget_->SetMergeModeTags (tags);
		}
		else
			Ui_.ItemsWidget_->CurrentChannelChanged (mapped);
	}

	void AggregatorTab::handleGroupChannels ()
	{
		if (XmlSettingsManager::Instance ()->property ("GroupChannelsByTags").toBool ())
		{
			FlatToFolders_->SetSourceModel (ChannelsFilterModel_);
			Ui_.Feeds_->setModel (FlatToFolders_.get ());
		}
		else
		{
			FlatToFolders_->SetSourceModel (nullptr);
			Ui_.Feeds_->setModel (ChannelsFilterModel_);
		}
		connect (Ui_.Feeds_->selectionModel (),
				&QItemSelectionModel::currentChanged,
				this,
				&AggregatorTab::currentChannelChanged);
		Ui_.Feeds_->expandAll ();
	}

	void AggregatorTab::on_MergeItems__toggled (bool merge)
	{
		Ui_.ItemsWidget_->SetMergeMode (merge);
		XmlSettingsManager::Instance ()->setProperty ("MergeItems", merge);
	}
}
}
