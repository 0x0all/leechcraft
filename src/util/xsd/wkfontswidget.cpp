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

#include "wkfontswidget.h"
#include <QTimer>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <util/sll/qtutil.h>
#include <util/sll/slotclosure.h>
#include <interfaces/iwkfontssettable.h>
#include "ui_wkfontswidget.h"
#include "massfontchangedialog.h"

namespace LeechCraft
{
namespace Util
{
	WkFontsWidget::WkFontsWidget (BaseSettingsManager *bsm, QWidget *parent)
	: QWidget { parent }
	, Ui_ { std::make_shared<Ui::WkFontsWidget> () }
	, BSM_ { bsm }
	{
		Ui_->setupUi (this);

		Family2Chooser_ [IWkFontsSettable::FontFamily::StandardFont] = Ui_->StandardChooser_;
		Family2Chooser_ [IWkFontsSettable::FontFamily::FixedFont] = Ui_->FixedChooser_;
		Family2Chooser_ [IWkFontsSettable::FontFamily::SerifFont] = Ui_->SerifChooser_;
		Family2Chooser_ [IWkFontsSettable::FontFamily::SansSerifFont] = Ui_->SansSerifChooser_;
		Family2Chooser_ [IWkFontsSettable::FontFamily::CursiveFont] = Ui_->CursiveChooser_;
		Family2Chooser_ [IWkFontsSettable::FontFamily::FantasyFont] = Ui_->FantasyChooser_;

		Family2Name_ [IWkFontsSettable::FontFamily::StandardFont] = "StandardFont";
		Family2Name_ [IWkFontsSettable::FontFamily::FixedFont] = "FixedFont";
		Family2Name_ [IWkFontsSettable::FontFamily::SerifFont] = "SerifFont";
		Family2Name_ [IWkFontsSettable::FontFamily::SansSerifFont] = "SansSerifFont";
		Family2Name_ [IWkFontsSettable::FontFamily::CursiveFont] = "CursiveFont";
		Family2Name_ [IWkFontsSettable::FontFamily::FantasyFont] = "FantasyFont";

		ResetFontChoosers ();

		for (const auto& pair : Util::Stlize (Family2Chooser_))
			new Util::SlotClosure<Util::NoDeletePolicy>
			{
				[this, pair] { PendingFontChanges_ [pair.first] = pair.second->GetFont (); },
				pair.second,
				SIGNAL (fontChanged (QFont)),
				this
			};

		Size2Spinbox_ [IWkFontsSettable::FontSize::DefaultFontSize] = Ui_->SizeDefault_;
		Size2Spinbox_ [IWkFontsSettable::FontSize::DefaultFixedFontSize] = Ui_->SizeFixedWidth_;
		Size2Spinbox_ [IWkFontsSettable::FontSize::MinimumFontSize] = Ui_->SizeMinimum_;

		Size2Name_ [IWkFontsSettable::FontSize::DefaultFontSize] = "FontSize";
		Size2Name_ [IWkFontsSettable::FontSize::DefaultFixedFontSize] = "FixedFontSize";
		Size2Name_ [IWkFontsSettable::FontSize::MinimumFontSize] = "MinimumFontSize";

		ResetSizeChoosers ();

		for (const auto& pair : Util::Stlize (Size2Spinbox_))
			new Util::SlotClosure<Util::NoDeletePolicy>
			{
				[this, pair] { PendingSizeChanges_ [pair.first] = pair.second->value (); },
				pair.second,
				SIGNAL (valueChanged (int)),
				this
			};

		ResetZoom ();

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { IsFontZoomDirty_ = true; },
			Ui_->Zoom_,
			SIGNAL (valueChanged (int)),
			this
		};
	}

	void WkFontsWidget::SetFontZoomTooltip (const QString& label)
	{
		Ui_->Zoom_->setToolTip (label);
	}

	void WkFontsWidget::RegisterSettable (IWkFontsSettable *settable)
	{
		Settables_ << settable;
		connect (settable->GetQObject (),
				&QObject::destroyed,
				[this, settable] { Settables_.removeOne (settable); });

		for (const auto& pair : Util::Stlize (Family2Chooser_))
			settable->SetFontFamily (pair.first, pair.second->GetFont ());

		for (const auto& pair : Util::Stlize (Size2Spinbox_))
			settable->SetFontSize (pair.first, pair.second->value ());

		settable->SetFontSizeMultiplier (Ui_->Zoom_->value () / 100.);
	}

	void WkFontsWidget::SetSize (IWkFontsSettable::FontSize type, int size)
	{
		Size2Spinbox_ [type]->setValue (size);
		PendingSizeChanges_ [type] = size;

		QTimer::singleShot (1000, this, [this] { ApplyPendingSizeChanges (); });
	}

	void WkFontsWidget::ResetFontChoosers ()
	{
		for (const auto& pair : Util::Stlize (Family2Chooser_))
		{
			const auto& option = Family2Name_ [pair.first];
			pair.second->SetFont (BSM_->property (option).value<QFont> ());
		}
	}

	void WkFontsWidget::ResetSizeChoosers ()
	{
		for (const auto& pair : Util::Stlize (Size2Spinbox_))
		{
			const auto& option = Size2Name_ [pair.first];
			pair.second->setValue (BSM_->Property (option, 10).toInt ());
		}
	}

	void WkFontsWidget::ResetZoom ()
	{
		const auto factor = BSM_->Property ("FontSizeMultiplier", 1).toDouble ();
		Ui_->Zoom_->setValue (factor * 100);
	}

	void WkFontsWidget::ApplyPendingSizeChanges ()
	{
		for (const auto& pair : Util::Stlize (PendingSizeChanges_))
		{
			BSM_->setProperty (Size2Name_ [pair.first], pair.second);
			emit sizeChanged (pair.first, pair.second);

			for (const auto settable : Settables_)
				settable->SetFontSize (pair.first, pair.second);
		}

		PendingSizeChanges_.clear ();
	}

	void WkFontsWidget::on_ChangeAll__released ()
	{
		QHash<QString, QList<IWkFontsSettable::FontFamily>> families;
		for (const auto& pair : Util::Stlize (Family2Chooser_))
			families [pair.second->GetFont ().family ()] << pair.first;

		const auto& stlized = Util::Stlize (families);
		const auto& maxElem = std::max_element (stlized.begin (), stlized.end (),
				[] (auto left, auto right) { return left.second.size () < right.second.size (); });

		const auto dialog = new MassFontChangeDialog { maxElem->first, maxElem->second, this };

		dialog->show ();
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[dialog, this]
			{
				dialog->deleteLater ();
				if (dialog->result () == QDialog::Rejected)
					return;

				const auto& font = dialog->GetFont ();
				for (const auto family : dialog->GetFamilies ())
				{
					PendingFontChanges_ [family] = font;
					Family2Chooser_ [family]->SetFont (font);
				}
			},
			dialog,
			SIGNAL (finished (int)),
			dialog
		};
	}

	void WkFontsWidget::accept ()
	{
		ApplyPendingSizeChanges ();

		for (const auto& pair : Util::Stlize (PendingFontChanges_))
		{
			BSM_->setProperty (Family2Name_ [pair.first], pair.second);
			emit fontChanged (pair.first, pair.second);

			for (const auto settable : Settables_)
				settable->SetFontFamily (pair.first, pair.second);
		}

		if (IsFontZoomDirty_)
		{
			const auto factor = Ui_->Zoom_->value () / 100.;

			BSM_->setProperty ("FontSizeMultiplier", factor);
			emit sizeMultiplierChanged (factor);

			for (const auto settable : Settables_)
				settable->SetFontSizeMultiplier (factor);
		}

		PendingFontChanges_.clear ();
		IsFontZoomDirty_ = false;
	}

	void WkFontsWidget::reject ()
	{
		ResetFontChoosers ();
		ResetSizeChoosers ();
		ResetZoom ();

		PendingFontChanges_.clear ();
		PendingSizeChanges_.clear ();
		IsFontZoomDirty_ = false;
	}
}
}
