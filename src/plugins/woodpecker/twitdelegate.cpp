/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Slava Barinov <rayslava@gmail.com>
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

#include "twitdelegate.h"
#include <QDebug>
#include <QUrl>
#include <QPushButton>
#include <QApplication>
#include <QMouseEvent>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QRectF>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/structures.h>
#include <util/util.h>
#include "core.h"
#include "tweet.h"

namespace LeechCraft
{
namespace Woodpecker
{
	TwitDelegate::TwitDelegate (QObject *parent)
	: QAbstractItemDelegate (parent)
	{
		m_parent = parent;
	}

	QObject* TwitDelegate::parent ()
	{
		return m_parent;
	}

	void TwitDelegate::paint (QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
	{
		QRect r = option.rect;

		//Color: #C4C4C4
		QPen linePen (QColor::fromRgb (211,211,211), 1, Qt::SolidLine);

		//Color: #005A83
		QPen lineMarkedPen (QColor::fromRgb (0,90,131), 1, Qt::SolidLine);

		//Color: #333
		QPen fontPen (QColor::fromRgb (51,51,51), 1, Qt::SolidLine);

		//Color: #Link
		QPen linkFontPen (QColor::fromRgb (51,51,255), 1, Qt::SolidLine);

		//Color: #fff
		QPen fontMarkedPen (Qt::white, 1, Qt::SolidLine);

		QFont mainFont ("Dejavu Sans", 10, QFont::Normal);

		if (option.state & QStyle::State_Selected)
		{
			QLinearGradient gradientSelected (r.left (),r.top (),r.left (),r.height ()+r.top ());
			gradientSelected.setColorAt (0.0, QColor::fromRgb (119,213,247));
			gradientSelected.setColorAt (0.9, QColor::fromRgb (27,134,183));
			gradientSelected.setColorAt (1.0, QColor::fromRgb (0,120,174));
			painter->setBrush (gradientSelected);
			painter->drawRect (r);

			// Border
			painter->setPen (lineMarkedPen);
			painter->drawLine (r.topLeft (),r.topRight ());
			painter->drawLine (r.topRight (),r.bottomRight ());
			painter->drawLine (r.bottomLeft (),r.bottomRight ());
			painter->drawLine (r.topLeft (),r.bottomLeft ());

			painter->setPen (fontMarkedPen);

		} 
		else 
		{
			// Background
			// Alternating colors
			painter->setBrush ((index.row () % 2) ? Qt::white : QColor (252,252,252));
			painter->drawRect (r);

			// border
			painter->setPen (linePen);
			painter->drawLine (r.topLeft (),r.topRight ());
			painter->drawLine (r.topRight (),r.bottomRight ());
			painter->drawLine (r.bottomLeft (),r.bottomRight ());
			painter->drawLine (r.topLeft (),r.bottomLeft ());

			painter->setPen (fontPen);
		}

		// Get title, description and icon
		QIcon ic = QIcon (qvariant_cast<QPixmap> (index.data (Qt::DecorationRole)));
		auto current_tweet = index.data (Qt::UserRole).value<std::shared_ptr<Tweet>> ();
		if (!current_tweet) 
		{
			qDebug () << "Can't recieve twit";
			return;
		}
		QString author = current_tweet->author ()->username ();
		qulonglong id = current_tweet->id ();
		QString time = current_tweet->dateTime ().toString ();
		QTextDocument* doc = current_tweet->getDocument ();

		int imageSpace = 50;
		if (!ic.isNull ()) 
		{
			// Icon
			r = option.rect.adjusted (5, 10, -10, -10);
			ic.paint (painter, r, Qt::AlignVCenter | Qt::AlignLeft);
		}

		// Text
		r = option.rect.adjusted (imageSpace, 4, -10, -22);
		painter->setFont (mainFont);
		doc->setTextWidth (r.width ());
		painter->save ();
		painter->translate (r.left (), r.top ());
		doc->drawContents (painter, r.translated (-r.topLeft ()));
		painter->restore ();

		// Author
		r = option.rect.adjusted (imageSpace + 4, 30, -10, 0);
		auto author_rect = std::unique_ptr<QRect> (new QRect (r.left (), r.bottom () - painter->fontMetrics ().height () - 8, painter->fontMetrics ().width (author), r.height ()));
		painter->setPen (linkFontPen);
		painter->setFont (mainFont);
		painter->drawText (* (author_rect), Qt::AlignLeft, author, &r);
		painter->setPen (fontPen);

		// Time
		r = option.rect.adjusted (imageSpace, 30, -10, 0);
		painter->setFont (mainFont);
		painter->drawText (r.right () - painter->fontMetrics ().width (time), r.bottom () - painter->fontMetrics ().height () - 8, r.width (), r.height (), Qt::AlignLeft, time, &r);
		painter->setPen (linePen);
	}

	QSize TwitDelegate::sizeHint (const QStyleOptionViewItem & option, const QModelIndex & index) const
	{
		return QSize (200, 60); // very dumb value
	}

	TwitDelegate::~TwitDelegate ()
	{
	}

	bool TwitDelegate::editorEvent (QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
	{
		QListWidget*  parent_widget = qobject_cast<QListWidget*> (m_parent);

		const int imageSpace = 50;
		if (event->type () == QEvent::MouseButtonRelease) 
		{
			QMouseEvent *me = (QMouseEvent*)event;
			if (me)
			{
				auto current_tweet = index.data (Qt::UserRole).value<std::shared_ptr<Tweet>> ();
				auto position = (me->pos () - option.rect.adjusted (imageSpace + 14, 4, 0, -22).topLeft ());

				qDebug () << "Coordinates: " << position.x () << ", " << position.y ();

				QTextDocument* textDocument = current_tweet->getDocument ();
				int textCursorPosition =
					textDocument->documentLayout ()->hitTest (position, Qt::FuzzyHit);
				QChar character (textDocument->characterAt (textCursorPosition));
				QString string;
				string.append (character);

				qDebug () << __FILE__ << __LINE__ << "Mouse pressed on letter " << string;

				auto anchor = textDocument->documentLayout ()->anchorAt (position);

				if (parent_widget)
					if (!anchor.isEmpty ())
					{
						Entity url = Util::MakeEntity (QUrl (anchor), QString (), OnlyHandle | FromUserInitiated, QString ());
						Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (url);
					}

			}
		}
		else if (event->type () == QEvent::MouseMove)
		{
			qDebug () << "Mouse moved";
			QMouseEvent *me = (QMouseEvent*)event;
			if (me)
			{
				auto current_tweet = index.data (Qt::UserRole).value<std::shared_ptr<Tweet>> ();
				auto position = (me->pos () - option.rect.adjusted (imageSpace + 14, 4, 0, -22).topLeft ());

				qDebug () << "Move coordinates: " << position.x () << ", " << position.y ();
				QTextDocument* textDocument = current_tweet->getDocument ();
				auto anchor = textDocument->documentLayout ()->anchorAt (position);

				if (parent_widget) 
				{
					if (!anchor.isEmpty ())
						parent_widget->setCursor (Qt::PointingHandCursor);
					else
						parent_widget->unsetCursor ();
				}
			}
		}
		return QAbstractItemDelegate::editorEvent (event, model, option, index);
	}
}
}
