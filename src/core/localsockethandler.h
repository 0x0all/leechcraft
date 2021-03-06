/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <boost/program_options.hpp>
#include <QLocalServer>

namespace LC
{
	struct Entity;

	class LocalSocketHandler : public QObject
	{
		Q_OBJECT

		const std::unique_ptr<QLocalServer> Server_;
	public:
		LocalSocketHandler ();
	private slots:
		void handleNewLocalServerConnection ();
		void pullCommandLine ();
	private:
		void DoLine (const boost::program_options::variables_map&, const QString&);
	signals:
		void gotEntity (const LC::Entity&);
	};
};
