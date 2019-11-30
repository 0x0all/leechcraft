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

#include <atomic>
#include <QString>
#include <vmime/net/tracer.hpp>

namespace LC
{
namespace Snails
{
	class AccountLogger;

	class Tracer : public vmime::net::tracer
	{
		std::atomic<uint64_t>& Sent_;
		std::atomic<uint64_t>& Received_;

		const QString Context_;
		const int ConnId_;
		const std::shared_ptr<AccountLogger> AccLogger_;
	public:
		Tracer (std::atomic<uint64_t>&, std::atomic<uint64_t>&,
		        const QString&, int, const std::shared_ptr<AccountLogger>&);

		void traceReceiveBytes (const vmime::size_t count, const vmime::string& state) override;
		void traceSendBytes (const vmime::size_t count, const vmime::string& state) override;
		void traceReceive (const vmime::string& line) override;
		void traceSend (const vmime::string& line) override;
	};
}
}
