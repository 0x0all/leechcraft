/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QMetaType>

namespace LC
{
namespace Liznoo
{
	struct BatteryInfo
	{
		QString ID_;

		char Percentage_ = 0;

		/** Time until battery is fully charged in seconds, or 0 if
		 * battery isn't charging.
		 */
		qlonglong TimeToFull_;
		qlonglong TimeToEmpty_;
		double Voltage_ = 0;

		double Energy_ = 0;
		double EnergyFull_ = 0;
		double DesignEnergyFull_ = 0;
		double EnergyRate_ = 0;

		QString Technology_;

		double Temperature_;

		int CyclesCount_ = 0;

		void Dump ();
	};
}
}

Q_DECLARE_METATYPE (LC::Liznoo::BatteryInfo)
