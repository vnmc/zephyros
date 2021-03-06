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


#import <Cocoa/Cocoa.h>

#import "base/ZPYAppDelegate.h"
#import "base/ZPYMenuHandler.h"
#import "base/types.h"

#import "zephyros.h"


@implementation ZPYMenuHandler

- (id) init
{
    self = [super init];
    return self;
}


/**
 * Invoked when a menu has been clicked.
 */
- (IBAction) menuItemSelected: (id) sender
{
    NSString *commandId = [sender valueForKey: @"commandId"];
    
    if (commandId == nil || [commandId isEqualToString: @""])
        return;
    
#ifndef APPSTORE
    if ([commandId isEqualToString: @MENUCOMMAND_ENTER_LICENSE])
        Zephyros::GetLicenseManager()->ShowEnterLicenseDialog();
    else if ([commandId isEqualToString: @MENUCOMMAND_PURCHASE_LICENSE])
        Zephyros::GetLicenseManager()->OpenPurchaseLicenseURL();
    else
#endif
    {
        Zephyros::JavaScript::Array args = Zephyros::JavaScript::CreateArray();
        args->SetString(0, String([commandId UTF8String]));
        Zephyros::GetNativeExtensions()->GetClientExtensionHandler()->InvokeCallbacks(TEXT("onMenuCommand"), args);
    }
}


@end
