//
//  app_mac.mm
//  Zephyros
//
//  Created by Matthias Christen on 10.12.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

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