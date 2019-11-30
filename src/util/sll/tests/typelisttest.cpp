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

#include "typelisttest.h"
#include <QtTest>
#include <typelist.h>
#include <typelevel.h>

QTEST_MAIN (LC::Util::TypelistTest)

namespace LC
{
namespace Util
{
	void TypelistTest::testHasTypeTrue ()
	{
		static_assert (HasType<struct Foo> (Typelist<struct Bar, struct Baz, struct Foo> {}), "test failed");
	}

	void TypelistTest::testHasTypeFalse ()
	{
		static_assert (!HasType<struct Foo> (Typelist<struct Bar, struct Baz, struct Qux> {}), "test failed");
	}

	template<typename T>
	using IsVoid_t = std::is_same<T, void>;

	void TypelistTest::testFilter ()
	{
		using List_t = Typelist<struct Foo, struct Bar, void, void, int, double, void>;
		using Expected_t = Typelist<struct Foo, struct Bar, int, double>;
		using Removed_t = Typelist<void, void, void>;

		static_assert (std::is_same<Removed_t, Filter_t<IsVoid_t, List_t>>::value, "test failed");
		static_assert (std::is_same<Expected_t, Filter_t<Not<IsVoid_t>::Result_t, List_t>>::value, "test failed");
	}
}
}
