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


#import "base/app.h"
#import "base/ZPYAppDelegate.h"


namespace Zephyros {
namespace App {
    
    
void Quit()
{
    [NSApp terminate: [NSApp delegate]];
}
    
void QuitMessageLoop()
{
    App::Quit();
}
    
void Alert(String title, String msg, AlertStyle style)
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = [NSString stringWithUTF8String: title.c_str()];
    alert.informativeText = [NSString stringWithUTF8String: msg.c_str()];

    switch (style)
    {
    case App::AlertStyle::AlertInfo:
        alert.alertStyle = NSInformationalAlertStyle;
        break;
    case App::AlertStyle::AlertWarning:
        alert.alertStyle = NSWarningAlertStyle;
        break;
    case App::AlertStyle::AlertError:
        alert.alertStyle = NSCriticalAlertStyle;
        break;
    }
        
    [alert runModal];
}
    
void BeginWait()
{
}
    
void EndWait()
{
}

void Log(String msg)
{
    NSLog(@"%@", [NSString stringWithUTF8String: msg.c_str()]);
}
    
void SetMenuItemStatuses(JavaScript::Object items)
{
    NSMutableDictionary *statuses = [[NSMutableDictionary alloc] init];
        
    JavaScript::KeyList keys;
    items->GetKeys(keys);
    for (JavaScript::KeyType commandId : keys)
        [statuses setObject: [NSNumber numberWithInt: items->GetInt(commandId)] forKey: [NSString stringWithUTF8String: String(commandId).c_str()]];
        
    [(ZPYAppDelegate*) [NSApp delegate] setMenuItemStatuses: statuses];
}

} // namespace App
} // namespace Zephyros