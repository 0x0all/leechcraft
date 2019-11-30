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

#pragma once

#include <QObject>
#include <QMap>
#include <QPointF>
#include "structures.h"

namespace LC
{
namespace CpuLoad
{
	struct LoadTypeInfo;

	class CpuLoadProxyObj : public QObject
	{
		Q_OBJECT

		Q_PROPERTY (double ioPercentage READ GetIoPercentage NOTIFY percentagesChanged)
		Q_PROPERTY (double lowPercentage READ GetLowPercentage NOTIFY percentagesChanged)
		Q_PROPERTY (double mediumPercentage READ GetMediumPercentage NOTIFY percentagesChanged)
		Q_PROPERTY (double highPercentage READ GetHighPercentage NOTIFY percentagesChanged)

		Q_PROPERTY (QList<QPointF> ioHist READ GetIoHist NOTIFY histChanged)
		Q_PROPERTY (QList<QPointF> lowHist READ GetLowHist NOTIFY histChanged)
		Q_PROPERTY (QList<QPointF> mediumHist READ GetMediumHist NOTIFY histChanged)
		Q_PROPERTY (QList<QPointF> highHist READ GetHighHist NOTIFY histChanged)

		QMap<LoadPriority, LoadTypeInfo> Infos_;
		QMap<LoadPriority, QList<double>> History_;
	public:
		CpuLoadProxyObj (const QMap<LoadPriority, LoadTypeInfo>&);

		void Set (const QMap<LoadPriority, LoadTypeInfo>&);

		double GetIoPercentage () const;
		double GetLowPercentage () const;
		double GetMediumPercentage () const;
		double GetHighPercentage () const;

		QList<QPointF> GetIoHist () const;
		QList<QPointF> GetLowHist () const;
		QList<QPointF> GetMediumHist () const;
		QList<QPointF> GetHighHist () const;
	private:
		QList<QPointF> GetHist (LoadPriority) const;
	public slots:
		int getMaxX () const;
	signals:
		void percentagesChanged ();
		void histChanged ();
	};
}
}
