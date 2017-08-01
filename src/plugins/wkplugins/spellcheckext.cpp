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

#include "spellcheckext.h"
#include <QTextBoundaryFinder>
#include <interfaces/core/ipluginsmanager.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace WKPlugins
{
	namespace
	{
		QList<ISpellChecker_ptr> GetSpellcheckers (const ICoreProxy_ptr& proxy)
		{
			const auto ipm = proxy->GetPluginsManager ();

			QList<ISpellChecker_ptr> result;
			for (const auto scProvider : ipm->GetAllCastableTo<ISpellCheckProvider*> ())
				if (const auto sc = scProvider->CreateSpellchecker ())
					result << sc;
			return result;
		}
	}

	SpellcheckerExt::SpellcheckerExt (const ICoreProxy_ptr& proxy)
	: Proxy_ { proxy }
	, Spellcheckers_ { GetSpellcheckers (proxy) }
	{
	}

	bool SpellcheckerExt::isContinousSpellCheckingEnabled () const
	{
		return ContinousEnabled_ && !Spellcheckers_.isEmpty ();
	}

	void SpellcheckerExt::toggleContinousSpellChecking ()
	{
		ContinousEnabled_ = !ContinousEnabled_;
	}

	void SpellcheckerExt::learnWord (const QString& word)
	{
		for (const auto& sc : Spellcheckers_)
			sc->LearnWord (word);
	}

	void SpellcheckerExt::ignoreWordInSpellDocument (const QString& word)
	{
		Ignored_ << word;
	}

	namespace
	{
		bool IsWordStart (QTextBoundaryFinder::BoundaryReasons reasons,
				QTextBoundaryFinder::BoundaryType type)
		{
			return reasons & QTextBoundaryFinder::StartOfItem &&
					type & QTextBoundaryFinder::Word;
		}

		bool IsWordEnd (QTextBoundaryFinder::BoundaryReasons reasons,
				QTextBoundaryFinder::BoundaryType type)
		{
			return reasons & QTextBoundaryFinder::EndOfItem &&
					type & QTextBoundaryFinder::Word;
		}
	}

	void SpellcheckerExt::checkSpellingOfString (const QString& word,
			int *misspellingLocation, int *misspellingLength)
	{
		if (!misspellingLength || !misspellingLocation)
			return;

		*misspellingLocation = 0;
		*misspellingLength = 0;

		QTextBoundaryFinder finder { QTextBoundaryFinder::Word, word };
		bool isInWord = false;
		int start = -1;
		int end = -1;
		do
		{
			const auto reasons = finder.boundaryReasons ();

			if (IsWordStart (reasons, finder.type ()))
			{
				start = finder.position ();
				isInWord = true;
			}

			if (isInWord && IsWordEnd (reasons, finder.type ()))
			{
				end = finder.position ();

				const auto& str = word.mid (start, end - start);
				if (std::none_of (Spellcheckers_.begin (), Spellcheckers_.end (),
						[&str] (const ISpellChecker_ptr& sc) { return sc->IsCorrect (str); }))
				{
					*misspellingLocation = start;
					*misspellingLength = end - start;
					return;
				}

				isInWord = false;
			}
		}
		while (finder.toNextBoundary () > 0);
	}

	QString SpellcheckerExt::autoCorrectSuggestionForMisspelledWord (const QString& word)
	{
		if (!XmlSettingsManager::Instance ().property ("EnableAutocorrect").toBool ())
			return {};

		QStringList guesses;
		guessesForWord (word, {}, guesses);
		return guesses.value (0);
	}

	void SpellcheckerExt::guessesForWord (const QString& word, const QString&, QStringList& guesses)
	{
		for (const auto& sc : Spellcheckers_)
			if (!sc->IsCorrect (word))
				guesses += sc->GetPropositions (word);

		guesses.removeDuplicates ();
	}

	bool SpellcheckerExt::isGrammarCheckingEnabled ()
	{
		return false;
	}

	void SpellcheckerExt::toggleGrammarChecking ()
	{
	}

	void SpellcheckerExt::checkGrammarOfString (const QString&, QList<GrammarDetail>&, int*, int*)
	{
	}
}
}
