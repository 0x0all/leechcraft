/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include <QPair>
#include <QList>
#include <QRegExp>
#include <QVariantMap>

class QWidget;
class QString;
class QAction;
class QColor;

namespace LeechCraft
{
	/** Possible content types a text editor could handle.
	 */
	enum class ContentType
	{
		/** HTML content. The corresponding editor should provide WYSIWYG
		 * capabilities or at least some fancy highlighting for HTML.
		 */
		HTML,

		/** Well, plain text.
		 */
		PlainText
	};

	/** Enumeration for some standard editor actions.
	 */
	enum class EditorAction
	{
		Find,
		Replace,
		Bold,
		Italic,
		Underline,
		InsertLink,
		InsertImage,
		ToggleView
	};
}

/** @brief Interface to be implemented by editor widgets returned from
 * ITextEditor::GetTextEditor().
 *
 * HTML content can be considered as either HTML (taking all the markup
 * into account) or just plain text (without all the markup, just the
 * user-visible characters). The type parameter to GetContents() and
 * SetContents() methods toggles the way HTML content is considered.
 *
 * In general, ContentType::HTML type is used when the caller considers
 * the contents as HTML-enabled, and ContentType::PlainText otherwise.
 *
 * A text editor widget may also implement IAdvancedHTMLEditor if it
 * supports the functionality required by that interface.
 *
 * @sa IAdvancedHTMLEditor
 */
class Q_DECL_EXPORT IEditorWidget
{
public:
	virtual ~IEditorWidget () {}

	/** @brief Returns the editor contents for the given type.
	 *
	 * Returns the contents of this widget according to type.
	 *
	 * For example, for a WYSIWYG text editor widget with HTML contents
	 * @code <h1>header</h1> @endcode this function should return
	 * @code header @endcode for ContentType::PlainText (prerendering
	 * the text in a sense) and @code <h1>header</h1> @endcode for
	 * ContentType::HTML.
	 *
	 * @param[in] type How contents should be interpreted and returned.
	 * @return The editor contents interpreted according to type.
	 *
	 * @sa SetContents()
	 */
	virtual QString GetContents (LeechCraft::ContentType type) const = 0;

	/** @brief Sets contents of this widget interpreted as of the given
	 * type.
	 *
	 * Sets the contents of this widget to contents according to
	 * contents. If type is ContentType::HTML the contents should be
	 * interpreted as rich text, while if type is ContentType::PlainText
	 * the contents should be interpreted as plain text. That is,
	 * @code <h1>header</h1> @endcode should be shown as a big h1 header
	 * in the first case and as is, with tags, in the second.
	 *
	 * Another example is a string like @code <p> @endcode, which should
	 * be kept as is by an HTML editor if the type is ContentType::HTML,
	 * but converted to @code &lt;p> @endcode if the type is
	 * ContentType::PlainText.
	 *
	 * @param[in] contents The new contents of this widget.
	 * @param[in] type The type of the contents.
	 *
	 * @sa GetContents()
	 */
	virtual void SetContents (const QString& contents, LeechCraft::ContentType type) = 0;

	/** @brief Returns a standard editor action.
	 *
	 * Returns the given standard editor action or null if no such action
	 * is available. Ownership is <em>not</em> passed to the caller.
	 *
	 * @param[in] action The standard action to return.
	 * @return The action or null if not available.
	 */
	virtual QAction* GetEditorAction (LeechCraft::EditorAction action) = 0;

	/** @brief Adds a custom action to the editor toolbar, if any.
	 *
	 * This function adds a custom action to the editor toolbar, if the
	 * widget has any. Ownershit is <em>not</em> passed to the editor.
	 * The action can be later removed by RemoveAction().
	 *
	 * @param[in] action The custom action to add.
	 *
	 * @sa RemoveAction()
	 */
	virtual void AppendAction (QAction *action) = 0;

	virtual void AppendSeparator () = 0;

	/** @brief Removes a custom action from the editor.
	 *
	 * This function removes a custom action previously added by
	 * AppendAction().
	 *
	 * @param[in] action An action previously added by AppendAction().
	 *
	 * @sa AppendAction()
	 */
	virtual void RemoveAction (QAction *action) = 0;

	/** @brief Sets the background color of the \em editor to color.
	 *
	 * This function sets the background color of the \em editor of the
	 * given content-type to the given \em color.
	 *
	 * If the widget doesn't support the given content-type, this
	 * function does nothing.
	 *
	 * @param[in] color The new background color.
	 * @param[in] editor The editor to change color of.
	 */
	virtual void SetBackgroundColor (const QColor& color, LeechCraft::ContentType editor) = 0;
protected:
	/** @brief Notifies about contents changes.
	 *
	 * This signal is emitted each time contents of this editor widget
	 * change.
	 */
	virtual void textChanged () = 0;
};

class QDomElement;

/** @brief Interface for HTML/WYSIWYG editors with some advanced functionality.
 *
 * @section CustomTags Custom tags
 *
 * It is sometimes desirable to add support for custom tags not defined
 * in HTML standards, like LJ's user tags. This is done via the
 * SetCustomTags() method, which operates on a list of CustomTag
 * structures.
 *
 * Each custom tag consists of a tag name that is used to identify the
 * tag, and the function that makes HTML out of the given tag with the
 * given attributes and contents. The converter function is invoked each
 * time the web view is should be re-rendered after HTML edit has been
 * modified. See the documentation of CustomTag for more information.
 *
 * All of the above is implemented using XML parsing, so the document
 * should be a valid XML document as well.
 */
class Q_DECL_EXPORT IAdvancedHTMLEditor
{
public:
	virtual ~IAdvancedHTMLEditor () {}

	typedef QPair<QRegExp, QString> Replacement_t;
	typedef QList<Replacement_t> Replacements_t;

	/** @brief Describes a single custom tag.
	 */
	struct CustomTag
	{
		/** @brief The name of the custom tag, like \em lj.
		 */
		QString TagName_;

		/** @brief The converter of an instance of the tag to HTML.
		 *
		 * This function is invoked to convert an instance of the tag
		 * (passed as a QDomElement) to HTML. The conversion should be
		 * done in-place: the resulting HTML should be contained in the
		 * passed QDomElement.
		 *
		 * An example function that boldifies
		 * <code><lj user="$username"/></code>:
		 *
		 * \code
		 * [] (QDomElement& elem) -> void
		 * {
		 * 	const auto& user = elem.attribute ("user");
		 * 	elem.setTagName ("strong");
		 * 	elem.removeAttribute ("user");
		 * 	elem.appendChild (elem.ownerDocument ().createTextNode (user));
		 * }
		 * \endcode
		 */
		std::function<void (QDomElement&)> ToKnown_;

		/** @brief The converter of an instance of the tag from HTML.
		 *
		 * This function is invoked to convert an instance of the tag
		 * (passed as a QDomElement) from HTML. The conversion should be
		 * done in-place: the resulting XML should be contained in the
		 * passed QDomElement.
		 *
		 * An example function that turns back the boldified
		 * <code><lj user="$username"/></code> from ToKnown_:
		 *
		 * \code
		 * [] (QDomElement& elem) -> void
		 * {
		 * 	const auto& user = elem.text ();
		 * 	elem.setTagName ("lj");
		 * 	elem.setAttribute ("user", user);
		 *
		 * 	const auto& childNodes = elem.childNodes ();
		 * 	while (!childNodes.isEmpty ())
		 * 		elem.removeChild (childNodes.at (0));
		 * }
		 * \endcode
		 *
		 * One can leave this function unset, in this case the tag will
		 * marked as non-modifyable.
		 */
		std::function<void (QDomElement&)> FromKnown_;
	};
	typedef QList<CustomTag> CustomTags_t;

	/** @brief Inserts the given HTML at the current cursor position.
	 *
	 * This function is somewhat analogous to DOM's
	 * @code execCommand("insertHTML", ...) @endcode.
	 *
	 * @param[in] html The HTML to insert.
	 */
	virtual void InsertHTML (const QString& html) = 0;

	/** @brief Sets tags mapping for this editor widget.
	 *
	 * This function should be set before ITextEditor::SetContents().
	 *
	 * See the IAdvancedHTMLEditor class reference for more information
	 * about tags mappings.
	 *
	 * @param[in] rich2html Mappings for view -> source view conversion.
	 * @param[in] html2rich Mappings for source view -> view conversion.
	 */
	virtual void SetTagsMappings (const Replacements_t& rich2html, const Replacements_t& html2rich) = 0;

	/** @brief Adds support for custom tags not present in HTML standard.
	 *
	 * This function should be called before ITextEditor::SetContents().
	 *
	 * See the IAdvancedHTMLEditor class reference for more information
	 * about tags mappings.
	 *
	 * @param[in] tags The tags mapping.
	 */
	virtual void SetCustomTags (const CustomTags_t& tags) = 0;

	/** @brief Adds a custom action to wrap selected text into given tag.
	 *
	 * For example, to insert an action to wrap selected text into
	 * <code><span style="font-weight: bold" id="sometext">...</span></code>
	 * one should call this function like this:
	 * \code
	 * QVariantMap params;
	 * params ["style"] = "font-weight: bold";
	 * params ["id"] = "sometext";
	 * auto action = editor->AddInlineTagInserter ("span", params);
	 * action->setText ("Name of your action");
	 * // further customize the action
	 * \endcode
	 *
	 * @param[in] tagName The name of the tag to be inserted.
	 * @param[in] params The parameters of the tag.
	 */
	virtual QAction* AddInlineTagInserter (const QString& tagName, const QVariantMap& params) = 0;

	/** @brief Executes the given js in the context of the content.
	 *
	 * @param[in] js The JavaScript code to execute.
	 */
	virtual void ExecJS (const QString& js) = 0;
};

/** @brief Interface for plugins implementing a text editor component.
 *
 * If a plugin can provide a text editor widget for other plugins to use
 * it should implement this interface.
 *
 * For example, plugins like a blog client (Blogique) or mail client
 * (Snails) would use such editor widget to allow the user to write posts
 * or mails.
 */
class Q_DECL_EXPORT ITextEditor
{
public:
	virtual ~ITextEditor () {}

	/** @brief Whether this editor plugin supports editing the content type.
	 *
	 * Plain text editors should return true only for the
	 * ContentType::PlainText type, while WYSIWYG-enabled editors should
	 * return true for ContentType::HTML as well.
	 *
	 * @param[in] type The content type to query.
	 * @return Whether the plugin supports editing the given type.
	 */
	virtual bool SupportsEditor (LeechCraft::ContentType type) const = 0;

	/** @brief Creates and returns a new text editor for the given type.
	 *
	 * This function should create a new text editor widget implementing
	 * IEditorWidget for the given content type. If creation fails for
	 * some reason (like unsupported type) this function should return 0.
	 *
	 * It is generally OK to return a WYSIWYG-enabled editor for the
	 * ContentType::PlainText type as long as it supports editing plain
	 * text.
	 *
	 * @param[in] type The content type for which to create the editor.
	 * @return An editor widget implementing IEditorWidget or nullptr in
	 * case of failure.
	 */
	virtual QWidget* GetTextEditor (LeechCraft::ContentType type) = 0;
};

Q_DECLARE_INTERFACE (IEditorWidget, "org.Deviant.LeechCraft.IEditorWidget/1.0");
Q_DECLARE_INTERFACE (IAdvancedHTMLEditor, "org.Deviant.LeechCraft.IAdvancedHTMLEditor/1.0");
Q_DECLARE_INTERFACE (ITextEditor, "org.Deviant.LeechCraft.ITextEditor/1.0");
