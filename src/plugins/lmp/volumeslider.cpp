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

#include "volumeslider.h"
#include <interfaces/core/iiconthememanager.h>
#include "engine/output.h"
#include "core.h"

namespace LC
{
namespace LMP
{
	VolumeSlider::VolumeSlider (Output *out, QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Ui_.Slider_->setOrientation (Qt::Horizontal);
		Ui_.Slider_->setRange (0, 100);
		Ui_.Slider_->setValue (out->GetVolume () * 100);

		connect (Ui_.Slider_,
				SIGNAL (valueChanged (int)),
				out,
				SLOT (setVolume (int)));
		connect (out,
				SIGNAL (volumeChanged (int)),
				Ui_.Slider_,
				SLOT (setValue (int)));

		connect (out,
				SIGNAL (mutedChanged (bool)),
				this,
				SLOT (handleMuted (bool)));
		handleMuted (out->IsMuted ());

		connect (Ui_.MuteButton_,
				SIGNAL (released ()),
				out,
				SLOT (toggleMuted ()));
	}

	void VolumeSlider::handleMuted (bool muted)
	{
		const auto iconName = muted ? "player-volume-muted" : "player-volume";
		Ui_.MuteButton_->setIcon (Core::Instance ().GetProxy ()->
					GetIconThemeManager ()->GetIcon (iconName));
	}
}
}
