//
//  zephyros_webview.m
//  Zephyros
//
//  Created by Matthias Christen on 14.08.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import "base/zephyros_impl.h"
#import "base/webview/ZPYWebViewAppDelegate.h"


namespace Zephyros {
    
int InitApplication(int argc, const char** argv)
{
    // create the application and load the main menu
    NSApplication *application = [NSApplication sharedApplication];
    NSArray *tl;
    [[NSBundle mainBundle] loadNibNamed: @"MainMenu" owner: application topLevelObjects: &tl];

    // create the app delegate
    ZPYWebViewAppDelegate *appDelegate = [[ZPYWebViewAppDelegate alloc] init];
    [application setDelegate: appDelegate];
    
    // create the main window
    NSWindowController *mainWindowController = [[NSWindowController alloc] initWithWindowNibName: @"MainWindow" owner: appDelegate];
    [mainWindowController window];
    
    // run the application
    [application run];
    return 0;
}
    
} // namespace Zephyros