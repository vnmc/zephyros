/*******************************************************************************
 * Copyright (c) 2015-2017 Vanamco AG, http://www.vanamco.com
 *
 * The MIT License (MIT)
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * Matthias Christen, Vanamco AG
 *******************************************************************************/

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_frame.h"

#include "base/app.h"
#include "base/cef/client_handler.h"


namespace Zephyros {

void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    CEF_REQUIRE_UI_THREAD();

    // TODO: show window (if needed)
}

bool ClientHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event)
{
    GObject* window = G_OBJECT(gtk_widget_get_ancestor(App::GetMainHwnd(), GTK_TYPE_WINDOW));

    GdkKeymapKey k;
    k.keycode = event.native_key_code;
    k.group = 0;
    k.level = 0;

    return gtk_accel_groups_activate(window, gdk_keymap_lookup_key(gdk_keymap_get_default(), &k), (GdkModifierType) event.modifiers) ? true : false;
}

} // namespace Zephyros
