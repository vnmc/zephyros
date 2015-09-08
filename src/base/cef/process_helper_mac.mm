/*******************************************************************************
 * Copyright (c) 2015 Vanamco AG, http://www.vanamco.com
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


#include <Foundation/Foundation.h>

#include "lib/cef/include/cef_app.h"

#include "base/types.h"
#include "base/app.h"
#include "base/cef/client_app.h"


bool g_isWindowLoaded = false;
bool g_isWindowBeingLoaded = true;


namespace Zephyros {
namespace App {
    
CefWindowHandle GetMainHwnd()
{
    return NULL;
}

void QuitMessageLoop()
{
}
    
void Alert(String title, String msg, AlertStyle style)
{
}
    
void BeginWait()
{
}
    
void EndWait()
{
}
    
void Log(String msg)
{
    NSLog(@"%s", msg.c_str());
}
    
void SetMenuItemStatuses(CefRefPtr<CefDictionaryValue> items)
{
}
    
} // namespace App
} // namespace Zephyros
