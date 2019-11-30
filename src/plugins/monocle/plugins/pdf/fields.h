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
#include <interfaces/monocle/iformfield.h>

namespace Poppler
{
	class FormField;
	class FormFieldText;
	class FormFieldChoice;
	class FormFieldButton;
}

namespace LC
{
namespace Monocle
{
namespace PDF
{
	class Document;

	class FormField : public QObject
					, public IFormField
	{
		Q_OBJECT
		Q_INTERFACES (LC::Monocle::IFormField)

		std::shared_ptr<Poppler::FormField> BaseField_;
	protected:
		FormField (std::shared_ptr<Poppler::FormField>);
	public:
		int GetID () const;
		QRectF GetRect () const;
		QString GetName () const;
	};

	class FormFieldText final : public FormField
							  , public IFormFieldText
	{
		Q_INTERFACES (LC::Monocle::IFormField
				LC::Monocle::IFormFieldText)

		std::shared_ptr<Poppler::FormFieldText> Field_;
	public:
		FormFieldText (std::shared_ptr<Poppler::FormField>);

		FormType GetType () const;
		Qt::Alignment GetAlignment () const;

		QString GetText () const;
		void SetText (const QString&);
		Type GetTextType () const;

		int GetMaximumLength () const;
		bool IsPassword () const;
		bool IsRichText () const;
	};

	class FormFieldChoice final : public FormField
								, public IFormFieldChoice
	{
		Q_INTERFACES (LC::Monocle::IFormField
				LC::Monocle::IFormFieldChoice)

		std::shared_ptr<Poppler::FormFieldChoice> Field_;
	public:
		FormFieldChoice (std::shared_ptr<Poppler::FormField>);

		FormType GetType () const;
		Qt::Alignment GetAlignment () const;

		Type GetChoiceType () const;

		QStringList GetAllChoices () const;

		QList<int> GetCurrentChoices () const;
		void SetCurrentChoices (const QList<int>&);

		QString GetEditChoice () const;
		void SetEditChoice (const QString&);

		bool IsEditable () const;
	};

	class FormFieldButton final : public FormField
						        , public IFormFieldButton
	{
		Q_INTERFACES (LC::Monocle::IFormField
				LC::Monocle::IFormFieldButton)

		std::shared_ptr<Poppler::FormFieldButton> Field_;
		Document * const Doc_;
		QList<int> ButtonGroup_;
	public:
		FormFieldButton (std::shared_ptr<Poppler::FormField>, Document*);

		FormType GetType () const;
		Qt::Alignment GetAlignment () const;

		Type GetButtonType () const;

		QString GetCaption () const;

		bool IsChecked () const;
		void SetChecked (bool);

		QList<int> GetButtonGroup () const;

		void HandleActivated ();
	};
}
}
}
