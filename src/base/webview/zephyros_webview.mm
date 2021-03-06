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


#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import "base/webview/ZPYWebViewAppDelegate.h"


namespace Zephyros {
    
int RunApplication(int argc, char** argv)
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
