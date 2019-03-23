/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Andrey Batyiev
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

#include "imagemediawidget.h"
#include <QDebug>
#include "xeps/xmppbobmanager.h"
#include "xeps/xmppbobiq.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	ImageMediaWidget::ImageMediaWidget (const QPair<QString, QString>& uri,
			XMPPBobManager *manager, const QString& from, QWidget *parent)
	: QLabel (parent)
	{
		QByteArray data;
		if (uri.second.startsWith ("cid:"))
		{
			Cid_ = uri.second.mid (4);
			data = manager->Take (from, Cid_);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unhandled uri:"
					<< uri.second;

		if (!data.isNull ())
			setPixmap (QPixmap::fromImage (QImage::fromData (data)));
		else if (!Cid_.isEmpty ())
		{
			connect (manager,
					SIGNAL (bobReceived (const XMPPBobIq&)),
					this,
					SLOT (bobReceived (const XMPPBobIq&)));
			manager->RequestBob (from, Cid_);
		}
	}

	void ImageMediaWidget::bobReceived (const XMPPBobIq& bob)
	{
		if (bob.GetCid () == Cid_)
			setPixmap (QPixmap::fromImage (QImage::fromData (bob.GetData ())));
	}
}
}
}
