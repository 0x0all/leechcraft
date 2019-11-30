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

#ifndef PLUGINS_AZOTH_INTERFACES_IMEDIACALL_H
#define PLUGINS_AZOTH_INTERFACES_IMEDIACALL_H
#include <QtPlugin>
#include <QIODevice>

class QIODevice;
class QAudioFormat;

namespace LC
{
namespace Azoth
{
	class IMediaCall
	{
	public:
		virtual ~IMediaCall () {}

		enum Direction
		{
			DIn,
			DOut
		};

		enum State
		{
			SConnecting,
			SActive,
			SDisconnecting,
			SFinished
		};

		virtual Direction GetDirection () const = 0;

		virtual QString GetSourceID () const = 0;

		virtual void Accept () = 0;

		virtual void Hangup () = 0;

		virtual QIODevice* GetAudioDevice () = 0;

		virtual QAudioFormat GetAudioReadFormat () const = 0;

		virtual QAudioFormat GetAudioWriteFormat () const = 0;

		virtual QIODevice* GetVideoDevice () = 0;
	protected:
		virtual void stateChanged (State) = 0;

		virtual void audioModeChanged (QIODevice::OpenMode) = 0;

		virtual void readFormatChanged () = 0;

		virtual void writeFormatChanged () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IMediaCall,
		"org.Deviant.LeechCraft.Azoth.IMediaCall/1.0")

#endif
