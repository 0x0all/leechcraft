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

#include "notificationrule.h"
#include <QDataStream>
#include <QStringList>
#include <QtDebug>

namespace LC
{
namespace AdvancedNotifications
{
	bool operator== (const VisualParams&, const VisualParams&)
	{
		return true;
	}

	AudioParams::AudioParams ()
	{
	}

	AudioParams::AudioParams (const QString& fname)
	: Filename_ (fname)
	{
	}

	bool operator== (const AudioParams& ap1, const AudioParams& ap2)
	{
		return ap1.Filename_ == ap2.Filename_;
	}

	bool operator== (const TrayParams&, const TrayParams&)
	{
		return true;
	}

	CmdParams::CmdParams ()
	{
	}

	CmdParams::CmdParams (const QString& cmd, const QStringList& args)
	: Cmd_ (cmd)
	, Args_ (args)
	{
	}

	bool operator== (const CmdParams& cp1, const CmdParams& cp2)
	{
		return cp1.Args_ == cp2.Args_ &&
			cp1.Cmd_ == cp2.Cmd_;
	}

	NotificationRule::NotificationRule ()
	: Methods_ (NMNone)
	, IsEnabled_ (true)
	, IsSingleShot_ (false)
	{
	}

	NotificationRule::NotificationRule (const QString& name,
			const QString& cat, const QStringList& types)
	: Name_ (name)
	, Category_ (cat)
	, Types_ (types)
	, Methods_ (NMNone)
	, IsEnabled_ (true)
	, IsSingleShot_ (false)
	{
	}

	bool NotificationRule::IsNull () const
	{
		return Name_.isEmpty () ||
				Category_.isEmpty () ||
				Types_.isEmpty ();
	}

	QString NotificationRule::GetName () const
	{
		return Name_;
	}

	void NotificationRule::SetName (const QString& name)
	{
		Name_ = name;
	}

	QString NotificationRule::GetCategory () const
	{
		return Category_;
	}

	void NotificationRule::SetCategory (const QString& cat)
	{
		Category_ = cat;
	}

	QSet<QString> NotificationRule::GetTypes () const
	{
		return Types_.toSet ();
	}

	void NotificationRule::SetTypes (const QStringList& types)
	{
		Types_ = types;
	}

	NotificationMethods NotificationRule::GetMethods () const
	{
		return Methods_;
	}

	void NotificationRule::SetMethods (const NotificationMethods& methods)
	{
		Methods_ = methods;
	}

	void NotificationRule::AddMethod (NotificationMethod method)
	{
		Methods_ |= method;
	}

	FieldMatches_t NotificationRule::GetFieldMatches () const
	{
		return FieldMatches_;
	}

	VisualParams NotificationRule::GetVisualParams () const
	{
		return VisualParams_;
	}

	void NotificationRule::SetVisualParams (const VisualParams& params)
	{
		VisualParams_ = params;
	}

	AudioParams NotificationRule::GetAudioParams () const
	{
		return AudioParams_;
	}

	void NotificationRule::SetAudioParams (const AudioParams& params)
	{
		AudioParams_ = params;
	}

	TrayParams NotificationRule::GetTrayParams () const
	{
		return TrayParams_;
	}

	void NotificationRule::SetTrayParams (const TrayParams& params)
	{
		TrayParams_ = params;
	}

	CmdParams NotificationRule::GetCmdParams() const
	{
		return CmdParams_;
	}

	void NotificationRule::SetCmdParams (const CmdParams& params)
	{
		CmdParams_ = params;
	}

	bool NotificationRule::IsEnabled () const
	{
		return IsEnabled_;
	}

	void NotificationRule::SetEnabled (bool enabled)
	{
		IsEnabled_ = enabled;
	}

	bool NotificationRule::IsSingleShot () const
	{
		return IsSingleShot_;
	}

	void NotificationRule::SetSingleShot (bool shot)
	{
		IsSingleShot_ = shot;
	}

	void NotificationRule::SetFieldMatches (const FieldMatches_t& matches)
	{
		FieldMatches_ = matches;
	}

	void NotificationRule::AddFieldMatch (const FieldMatch& match)
	{
		FieldMatches_ << match;
	}

	QColor NotificationRule::GetColor () const
	{
		return Color_;
	}

	void NotificationRule::SetColor (const QColor& color)
	{
		Color_ = color;
	}

	void NotificationRule::Save (QDataStream& stream) const
	{
		stream << static_cast<quint8> (4)
			<< Name_
			<< Category_
			<< Types_
			<< static_cast<quint16> (Methods_)
			<< AudioParams_.Filename_
			<< CmdParams_.Cmd_
			<< CmdParams_.Args_
			<< IsEnabled_
			<< IsSingleShot_
			<< Color_
			<< static_cast<quint16> (FieldMatches_.size ());

		for (const auto& match : FieldMatches_)
			match.Save (stream);
	}

	void NotificationRule::Load (QDataStream& stream)
	{
		quint8 version = 0;
		stream >> version;
		if (version < 1 || version > 4)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return;
		}

		quint16 methods;
		stream >> Name_
			>> Category_
			>> Types_
			>> methods
			>> AudioParams_.Filename_;

		if (version >= 2)
			stream >> CmdParams_.Cmd_
				>> CmdParams_.Args_;

		if (version >= 3)
			stream >> IsEnabled_
				>> IsSingleShot_;
		else
		{
			IsEnabled_ = true;
			IsSingleShot_ = false;
		}

		if (version >= 4)
			stream >> Color_;

		Methods_ = static_cast<NotificationMethods> (methods);

		quint16 numMatches = 0;
		stream >> numMatches;

		for (int i = 0; i < numMatches; ++i)
		{
			FieldMatch match;
			match.Load (stream);
			FieldMatches_ << match;
		}
	}

	bool operator== (const NotificationRule& r1, const NotificationRule& r2)
	{
		return r1.GetMethods () == r2.GetMethods () &&
			r1.IsEnabled () == r2.IsEnabled () &&
			r1.IsSingleShot () == r2.IsSingleShot () &&
			r1.GetName () == r2.GetName () &&
			r1.GetCategory () == r2.GetCategory () &&
			r1.GetTypes () == r2.GetTypes () &&
			r1.GetFieldMatches () == r2.GetFieldMatches () &&
			r1.GetVisualParams () == r2.GetVisualParams () &&
			r1.GetAudioParams () == r2.GetAudioParams () &&
			r1.GetTrayParams () == r2.GetTrayParams () &&
			r1.GetCmdParams () == r2.GetCmdParams () &&
			r1.GetColor () == r2.GetColor ();
	}

	bool operator!= (const NotificationRule& r1, const NotificationRule& r2)
	{
		return !(r1 == r2);
	}

	namespace
	{
		template<typename T>
		void DebugSingle (const NotificationRule& r1, const NotificationRule& r2, T method)
		{
			const auto eq = (r1.*method) () == (r2.*method) ();
			qDebug () << eq;
			if (!eq)
				qDebug () << (r1.*method) () << (r2.*method) ();
		}
	}

	void DebugEquals (const NotificationRule& r1, const NotificationRule& r2)
	{
		qDebug () << Q_FUNC_INFO;
		DebugSingle (r1, r2, &NotificationRule::GetMethods);
		DebugSingle (r1, r2, &NotificationRule::IsEnabled);
		DebugSingle (r1, r2, &NotificationRule::IsSingleShot);
		DebugSingle (r1, r2, &NotificationRule::GetName);
		DebugSingle (r1, r2, &NotificationRule::GetCategory);
		DebugSingle (r1, r2, &NotificationRule::GetTypes);
		DebugSingle (r1, r2, &NotificationRule::GetFieldMatches);
		DebugSingle (r1, r2, &NotificationRule::GetVisualParams);
		DebugSingle (r1, r2, &NotificationRule::GetAudioParams);
		DebugSingle (r1, r2, &NotificationRule::GetTrayParams);
		DebugSingle (r1, r2, &NotificationRule::GetCmdParams);
		DebugSingle (r1, r2, &NotificationRule::GetColor);
	}
}
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::FieldMatch& match)
{
	dbg.nospace () << "FieldMatch (for: " << match.GetPluginID () << "; field: " << match.GetFieldName () << ")";
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::VisualParams&)
{
	dbg.nospace () << "VisualParams ()";
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::AudioParams& params)
{
	dbg.nospace () << "AudioParams (file: " << params.Filename_ << ")";
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::TrayParams&)
{
	dbg.nospace () << "TrayParams ()";
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::CmdParams& params)
{
	dbg.nospace () << "CmdParams (command: " << params.Cmd_ << "; args: " << params.Args_ << ")";
	return dbg.space ();
}
