/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "output.h"
#include <cmath>
#include <QtDebug>
#include <QTimer>
#include "../gstfix.h"
#include "path.h"
#include "../xmlsettingsmanager.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		/* Signature found on
		 * http://cgit.freedesktop.org/gstreamer/gst-plugins-base/commit/?id=de1db5ccbdc10a835a2dfdd5984892f3b0c9bcf4
		 *
		 * I love C language, it's freaking compile-time-safe and sane.
		 */
		gboolean CbVolumeChanged (GObject*, GParamSpec*, gpointer data)
		{
			auto output = static_cast<Output*> (data);
			const auto volume = output->GetVolume ();

			QMetaObject::invokeMethod (output,
					"volumeChanged",
					Q_ARG (int, std::round (volume * 100)));

			return true;
		}

		gboolean CbMuteChanged (GObject*, GParamSpec*, gpointer data)
		{
			auto output = static_cast<Output*> (data);
			const auto isMuted = output->IsMuted ();

			QMetaObject::invokeMethod (output,
					"mutedChanged",
					Q_ARG (bool, isMuted));

			return true;
		}
	}

	Output::Output (QObject *parent)
	: QObject (parent)
	, Bin_ (gst_bin_new ("audio_sink_bin"))
	, Volume_ (gst_element_factory_make ("volume", "volume"))
	{
		auto converter = gst_element_factory_make ("audioconvert", "convert");
		auto sink = gst_element_factory_make ("autoaudiosink", "audio_sink");

		gst_bin_add_many (GST_BIN (Bin_), Volume_, converter, sink, nullptr);
		gst_element_link_many (Volume_, converter, sink, nullptr);

		auto pad = gst_element_get_static_pad (Volume_, "sink");
		auto ghostPad = gst_ghost_pad_new ("sink", pad);
		gst_pad_set_active (ghostPad, TRUE);
		gst_element_add_pad (Bin_, ghostPad);
		gst_object_unref (pad);

		g_signal_connect (Volume_, "notify::volume", G_CALLBACK (CbVolumeChanged), this);
		g_signal_connect (Volume_, "notify::mute", G_CALLBACK (CbMuteChanged), this);

		const auto volume = XmlSettingsManager::Instance ().Property ("AudioVolume", 1).toDouble ();
		setVolume (volume);

		const auto isMuted = XmlSettingsManager::Instance ().Property ("AudioMuted", false).toBool ();
		g_object_set (G_OBJECT (Volume_), "mute", static_cast<gboolean> (isMuted), nullptr);
	}

	void Output::AddToPath (Path *path)
	{
		path->SetOutputBin (Bin_);
	}

	void Output::PostAdd (Path*)
	{
	}

	double Output::GetVolume () const
	{
		gdouble value = 1;
		g_object_get (G_OBJECT (Volume_), "volume", &value, nullptr);
		const auto exp = XmlSettingsManager::Instance ().property ("VolumeExponent").toDouble ();
		if (exp != 1)
			value = std::pow (value, 1 / exp);
		return value;
	}

	bool Output::IsMuted () const
	{
		gboolean value = false;
		g_object_get (G_OBJECT (Volume_), "mute", &value, nullptr);
		return value;
	}

	void Output::ScheduleSaveVolume ()
	{
		if (SaveVolumeScheduled_)
			return;

		SaveVolumeScheduled_ = true;
		QTimer::singleShot (1000,
				this,
				SLOT (saveVolume ()));
	}

	void Output::setVolume (double volume)
	{
		const auto exp = XmlSettingsManager::Instance ().property ("VolumeExponent").toDouble ();
		if (exp != 1)
			volume = std::pow (volume, exp);

		if (volume > 1)
			volume = 1;

		g_object_set (G_OBJECT (Volume_), "volume", static_cast<gdouble> (volume), nullptr);

		ScheduleSaveVolume ();
	}

	void Output::setVolume (int volume)
	{
		setVolume (volume / 100.);
	}

	void Output::toggleMuted ()
	{
		g_object_set (G_OBJECT (Volume_), "mute", static_cast<gboolean> (!IsMuted ()), nullptr);

		ScheduleSaveVolume ();
	}

	void Output::saveVolume ()
	{
		SaveVolumeScheduled_ = false;

		XmlSettingsManager::Instance ().setProperty ("AudioVolume", GetVolume ());
		XmlSettingsManager::Instance ().setProperty ("AudioMuted", IsMuted ());
	}
}
}
