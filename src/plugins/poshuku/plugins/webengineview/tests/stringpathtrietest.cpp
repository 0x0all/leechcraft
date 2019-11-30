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

#include "stringpathtrietest.h"
#include <QtTest>
#include <util/sll/prelude.h>
#include <stringpathtrie.h>

QTEST_MAIN (LC::Poshuku::WebEngineView::StringPathTrieTest)

namespace LC::Poshuku::WebEngineView
{
	using IntTrie = StringPathTrie<int>;

	QVector<QStringRef> AsRefs (const QVector<QString>& lst)
	{
		return Util::Map (lst, [] (const auto& str) { return QStringRef { &str }; });
	}

	void StringPathTrieTest::testEmptyTrie ()
	{
		IntTrie trie;

		QCOMPARE (trie.BestMatch (AsRefs ({ "foo", "bar", "baz" })), std::optional<int> {});
	}

	void StringPathTrieTest::testEmptyQuery ()
	{
		IntTrie trie;
		trie.Mark (AsRefs ({ "foo", "bar", "baz" }), 10);

		QCOMPARE (trie.BestMatch (AsRefs ({})), std::optional<int> { 10 });
	}

	void StringPathTrieTest::testExactMatchSingle ()
	{
		IntTrie trie;
		trie.Mark (AsRefs ({ "foo", "bar", "baz" }), 10);

		QCOMPARE (trie.BestMatch (AsRefs ({ "foo", "bar", "baz" })), std::optional<int> { 10 });
	}

	void StringPathTrieTest::testExactMatchOverwriteSingle ()
	{
		IntTrie trie;
		trie.Mark (AsRefs ({ "foo", "bar", "baz" }), 10);
		trie.Mark (AsRefs ({ "foo", "bar", "baz" }), 20);

		QCOMPARE (trie.BestMatch (AsRefs ({ "foo", "bar", "baz" })), std::optional<int> { 20 });
	}

	void StringPathTrieTest::testExactMatchMulti ()
	{
		IntTrie trie;
		trie.Mark (AsRefs ({ "foo", "bar", "baz1" }), 10);
		trie.Mark (AsRefs ({ "foo", "bar", "baz2" }), 20);

		QCOMPARE (trie.BestMatch (AsRefs ({ "foo", "bar", "baz1" })), std::optional<int> { 10 });
		QCOMPARE (trie.BestMatch (AsRefs ({ "foo", "bar", "baz2" })), std::optional<int> { 20 });
	}

	void StringPathTrieTest::testExactMatchParentPre ()
	{
		IntTrie trie;
		trie.Mark (AsRefs ({ "foo", "bar" }), 10);
		trie.Mark (AsRefs ({ "foo", "bar", "baz" }), 20);

		QCOMPARE (trie.BestMatch (AsRefs ({ "foo", "bar" })), std::optional<int> { 10 });
		QCOMPARE (trie.BestMatch (AsRefs ({ "foo", "bar", "baz" })), std::optional<int> { 20 });
	}

	void StringPathTrieTest::testExactMatchParentPost ()
	{
		IntTrie trie;
		trie.Mark (AsRefs ({ "foo", "bar", "baz" }), 20);
		trie.Mark (AsRefs ({ "foo", "bar" }), 10);

		QCOMPARE (trie.BestMatch (AsRefs ({ "foo", "bar" })), std::optional<int> { 10 });
		QCOMPARE (trie.BestMatch (AsRefs ({ "foo", "bar", "baz" })), std::optional<int> { 20 });
	}

	void StringPathTrieTest::testPartialMatchLongerQuery ()
	{
		IntTrie trie;
		trie.Mark (AsRefs ({ "foo" }), 20);
		trie.Mark (AsRefs ({ "foo", "bar" }), 10);

		QCOMPARE (trie.BestMatch (AsRefs ({ "foo", "bar", "baz" })), std::optional<int> { 10 });
	}

	void StringPathTrieTest::testPartialMatchShorterQuery ()
	{
		IntTrie trie;
		trie.Mark (AsRefs ({ "foo", "bar" }), 20);
		trie.Mark (AsRefs ({ "foo", "bar", "baz" }), 10);

		QCOMPARE (trie.BestMatch (AsRefs ({ "foo" })), std::optional<int> { 20 });
	}
}
