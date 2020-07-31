/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWebPage>
#include <util/xpc/basehookinterconnector.h>
#include <interfaces/core/ihookproxy.h>

class QWebView;

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	class PluginManager : public Util::BaseHookInterconnector
	{
		Q_OBJECT
	public:
		using BaseHookInterconnector::BaseHookInterconnector;
	signals:
		/** @brief Called whenever WebKit requests to navigate frame to
		 * the resource specified by request.
		 *
		 * The hook may modify the request, by using the
		 * IHookProxy::SetValue method with the name "request".
		 *
		 * If default handler is canceled, then its return value is
		 * converted to bool and is used as the return value of the
		 * QWebPage::acceptNavigationRequest() method, otherwise
		 * standard LeechCraft handler is used.
		 *
		 * Refer also to the documentation of the
		 * QWebPage::acceptNavigationRequest(), from which this hook is
		 * called.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The QWebPage from which this request originates.
		 * @param frame The frame to navigate, or null pointer if
		 * navigating to new window is requested.
		 * @param request The original network request.
		 * @param type The navigation type.
		 */
		void hookAcceptNavigationRequest (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QNetworkRequest request,
				QWebPage::NavigationType type);

		/** @brief Called inside QWebPage::chooseFile().
		 *
		 * This hook is called when the web content requests a a file
		 * name. If default handler is canceled, the proxy's return
		 * value is converted to QString and returned from the
		 * QWebPage::chooseFile() method, otherwise default handler is
		 * used. In the latter case, the hook may override the suggested
		 * filename by using IHookProxy::SetValue with the key
		 * "suggested" and value of type QString.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The web page that this request originated from.
		 * @param frame The frame that this request originated from.
		 * @param suggested The suggested filename.
		 */
		void hookChooseFile (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString suggested);

		/** @brief This hook is called whenever page contents change.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The web page whose contents changed.
		 */
		void hookContentsChanged (LC::IHookProxy_ptr proxy,
				QWebPage *page);

		/** @brief Called whenever an HTML object element is encountered.
		 *
		 * This hook is called inside QWebPage::createPlugin().
		 *
		 * If the default handler is canceled, the return value of the
		 * proxy is casted to QObject* and returned from the
		 * QWebPage::createPlugin(), otherwise the default handler is
		 * used. In the latter case, the hook may override the
		 * parameters by using IHookProxy::SetValue() with the following
		 * keys and values of corresponding types:
		 * - "clsid" of type QString
		 * - "url" of type QUrl
		 * - "params" of type QStringList
		 * - "values" of type QStringList
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The web page on which the element was
		 * encountered.
		 * @param clsid Original class ID.
		 * @param url Original URL of the object.
		 * @param params List of parameters to the plugin.
		 * @param values List of values of the parameters.
		 */
		void hookCreatePlugin (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QString clsid,
				QUrl url,
				QStringList params,
				QStringList values);

		/** @brief Called from QWebPage::createWindow().
		 *
		 * This hook is called whenever a new window of the given type
		 * should be created.
		 *
		 * If the default handler is canceled, the return value of the
		 * proxy is casted to QWebPage* and returned from the
		 * QWebPage::createWindow(). If the cast is successful, then
		 * the new window would be created, otherwise (or if a null
		 * pointer is returned) the window won't be created.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page originating the new window request.
		 * @param type The type of the new window.
		 */
		void hookCreateWindow (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebPage::WebWindowType type);

		/** @brief Called from QWebPage::databaseQuotaExceeded().
		 *
		 * @param proxy The standard hook proxy object.
		 * @param sourcePage The page originating the error.
		 * @param sourceFrame The frame with the site trying to store
		 * excessive data.
		 * @param databaseName The name of the database.
		 */
		void hookDatabaseQuotaExceeded (LC::IHookProxy_ptr proxy,
				QWebPage *sourcePage,
				QWebFrame *sourceFrame,
				QString databaseName);

		/** @brief Called from QWebPage::downloadRequested().
		 *
		 * If the default handler is canceled, nothing is done at all.
		 * Otherwise, the entity with the given request would be emitted
		 * to be downloaded by some other LeechCraft plugin. The hook
		 * may override the request by IHookProxy::SetValue() method
		 * with the name "request" and value of type QNetworkRequest.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param sourcePage The page originating the request.
		 * @param request The original download request.
		 */
		void hookDownloadRequested (LC::IHookProxy_ptr proxy,
				QWebPage *sourcePage,
				QNetworkRequest request);

		/** @brief Called from QWebPage::extension().
		 *
		 * If the default handler is canceled, the return value of the
		 * hook proxy is converted to bool and returned from the
		 * QWebPage::extension() method, otherwise the default handler
		 * is used.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page for which the extension should be
		 * handled.
		 * @param extension The extension.
		 * @param extensionOption The struct with options of the
		 * extension.
		 * @param extensionReturn The struct with return value of the
		 * extension.
		 *
		 * @sa hookSupportsExtension().
		 */
		void hookExtension (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebPage::Extension extension,
				const QWebPage::ExtensionOption* extensionOption,
				QWebPage::ExtensionReturn* extensionReturn);

		/** @brief Called when user wants to find the given text.s
		 *
		 * This hook is called when the user wants to find the given
		 * text on the given browserWidget. The hook may cancel the
		 * default handler or modify the text by calling the
		 * IHookProxy::SetValue() method with the "text" key and a value
		 * of type QString.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param browserWidget The browser widget originating the text
		 * find request.
		 * @param text The original text to be found by the user.
		 * @param findFlags The text find options.
		 */
		void hookFindText (LC::IHookProxy_ptr proxy,
				QObject *browserWidget,
				QString text,
				QWebPage::FindFlags findFlags);

		/** @brief Called whenever a new frame is created.
		 *
		 * This hook is called whenever a new frame is created on the
		 * given page.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page on which the frame is created.
		 * @param frameCreated The newly created frame.
		 */
		void hookFrameCreated (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frameCreated);

		/** @brief Called from QWebPage::geometryChangeRequested().
		 *
		 * This hook is called whenever the document want to change the
		 * page's position and size.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page that wants its position and size to be
		 * changed.
		 * @param rect The new position and size rect.
		 */
		void hookGeometryChangeRequested (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QRect rect);

		/** @brief Called from QWebPage::javaScriptAlert().
		 *
		 * This hook is called whenever a JS in the given frame wants to
		 * alert() the given message.
		 *
		 * The default implementation may be canceled inside the hook.
		 * Alternatively, the hook may override the message with
		 * IHookProxy::SetValue() with the key "message" and value of
		 * type QString.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page where the frame is.
		 * @param frame The frame containing the JS that wants to
		 * alert() a message.
		 * @param message The original message text.
		 */
		void hookJavaScriptAlert (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString message);

		/** @brief Called from QWebPage::javaScriptConfirm().
		 *
		 * This hook is called whenever a JS program in the given frame
		 * calls confirm() with the given message.
		 *
		 * If the default implementation is canceled from the hook, the
		 * proxy's return value is converted to bool and returned from
		 * the QWebPage::javaScriptConfirm() method. Otherwise, the hook
		 * may override the message with IHookProxy::SetValue() with the
		 * key "message and value of type QString.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page containing the frame.
		 * @param frame The frame containing the JS that wants to
		 * confirm() something.
		 * @param message The original message text.
		 */
		void hookJavaScriptConfirm (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString message);

		/** @brief Called from QWebPage::javaScriptConsoleMessage().
		 *
		 * This hook is called whenever a JS program wants to print a
		 * message to the web browser console.
		 *
		 * If the default implementation isn't canceled, the hook may
		 * override the message, line and sourceId using
		 * IHookProxy::SetValue() method with following keys and values,
		 * respectively:
		 * - "message" of type QString
		 * - "line" of type int
		 * - "sourceID" of type QString
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page containing the frame.
		 * @param message The original message text.
		 * @param line The line, if applicable.
		 * @param sourceId The ID of the source, if applicable.
		 */
		void hookJavaScriptConsoleMessage (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QString message,
				int line,
				QString sourceId);

		/** @brief Called from QWebPage::javaScriptPrompt().
		 *
		 * This hook is called whenever a JS program in the given frame
		 * wants user to input a string via prompt().
		 *
		 * If the default handler is canceled, the proxy's return value
		 * is converted to bool and returned from the
		 * QWebPage::javaScriptPrompt(), while the value set using
		 * IHookProxy::SetValue() with the key "result" is converted to
		 * QString and is considered to be the result. Otherwise, other
		 * parameters may be overridden in the hook in addition to
		 * "result", respectively:
		 * - "message" of type QString
		 * - "default" of type QString
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page containing the frame.
		 * @param frame The frame containing the JS that called
		 * prompt().
		 * @param message The original message text.
		 * @param defValue The default value suggested by the JS.
		 * @param resultString The result string.
		 */
		void hookJavaScriptPrompt (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString message,
				QString defValue,
				QString resultString);

		/** @brief Called when the global window object of the JS
		 * environment is cleared in the given frame.
		 *
		 * To ensure that objects added to a QWebFrame via the
		 * QWebFrame::addToJavaScriptWindowObject() method are
		 * accessible when loading new URLs, they should be readded
		 * in the corresponding hook.
		 *
		 * If the default handler is canceled, the "window.JSProxy" and
		 * "window.external" objects won't be added to the frame.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param sourcePage The page containing the frame.
		 * @param frame The frame whose window object is cleared.
		 */
		void hookJavaScriptWindowObjectCleared (LC::IHookProxy_ptr proxy,
				QWebPage *sourcePage,
				QWebFrame *frame);

		/** @brief Called whenever the given link is clicked.
		 *
		 * Seems like this hook is no longer used.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page where the link is clicked.
		 * @param url The URL of the link.
		 */
		void hookLinkClicked (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QUrl url);

		/** @brief Called whenever a given link is hovered by the mouse.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page where the link is hovered.
		 * @param link The URL of the link.
		 * @param title The HTML link element title, if specified in the
		 * markup.
		 * @param textContent The text within the HTML link element.
		 */
		void hookLinkHovered (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QString link,
				QString title,
				QString textContent);

		/** @brief Called when the given page finishes loading.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page that has finished loading.
		 * @param result Whether the page loaded successfully.
		 */
		void hookLoadFinished (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				bool result);

		/** @brief Called from QWebPage::supportsExtension().
		 *
		 * The hook may cancel the default implementation.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page whose contents triggered the query for
		 * the given extension support.
		 * @param extension The extension to be queried.
		 *
		 * @sa hookExtension().
		 */
		void hookSupportsExtension (LC::IHookProxy_ptr proxy,
				const QWebPage *page,
				QWebPage::Extension extension);

		/** @brief Called when the given page encounters unsupported
		 * content.
		 *
		 * The hook may choose to handle the reply and cancel the
		 * default implementation.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page that encountered the unsupported
		 * content.
		 * @param reply The QNetworkReply with the unsupported content.
		 */
		void hookUnsupportedContent (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QNetworkReply *reply);

		/** @brief Called when the given page begins constructing.
		 *
		 * This hook is useful for doing actions that should be done for
		 * each page constructed.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The QWebPage that begins constructing.
		 */
		void hookWebPageConstructionBegin (LC::IHookProxy_ptr proxy,
				QWebPage *page);

		/** @brief Called when the given page finishes constructing.
		 *
		 * This hook is analogous to hookWebPageConstructionBegin(), but
		 * it is called in the end of initialization process.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The QWebPage that finished constructing.
		 *
		 * @sa hookWebPageConstructionBegin().
		 */
		void hookWebPageConstructionEnd (LC::IHookProxy_ptr proxy,
				QWebPage *page);

		/** @brief Called from QWebPage::windowCloseRequested().
		 *
		 * This signal is called when the window close is requested by
		 * the page.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The web page originating the request.
		 */
		void hookWindowCloseRequested (LC::IHookProxy_ptr proxy,
				QWebPage *page);
	};
}
}
}
