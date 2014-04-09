//
//  app_webview.cpp
//  Zephyros
//
//  Created by Matthias Christen on 10.12.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#import "app.h"
#import "AppDelegate.h"


/////////////////////////////////////////////////////////////////////
// Global functions

namespace App {
    
    
void QuitMessageLoop()
{
    [NSApp terminate: [NSApp delegate]];
}
    
void Alert(String title, String msg, AlertStyle style)
{
    NSAlert *alert = [NSAlert alertWithMessageText: [NSString stringWithUTF8String: title.c_str()]
                                     defaultButton: @"OK"
                                   alternateButton: nil
                                       otherButton: nil
                         informativeTextWithFormat: @"%@", [NSString stringWithUTF8String: msg.c_str()]];
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

    
} // namespace App