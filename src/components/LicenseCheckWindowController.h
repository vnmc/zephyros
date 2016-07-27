/*******************************************************************************
 * Copyright (c) 2015-2016 Vanamco AG, http://www.vanamco.com
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


#ifndef Zephyros_LicenseCheckWindowController_h
#define Zephyros_LicenseCheckWindowController_h
#pragma once


#ifndef APPSTORE

#import <Cocoa/Cocoa.h>
#import "base/licensing.h"


@interface LicenseCheckWindowController : NSWindowController


@property (weak) IBOutlet NSTextField *dialogTitle;
@property (weak) IBOutlet NSTextField *dialogDescription;
@property (weak) IBOutlet NSImageView *imageDemo;
@property (weak) IBOutlet NSTextField *remainingTimeTitle;
@property (weak) IBOutlet NSTextField *remainingTimeDescription;
@property (weak) IBOutlet NSButton *purchaseLicense;
@property (weak) IBOutlet NSButton *enterLicenseKey;

@property (weak) IBOutlet NSTextField *enterLicenseKeyTitle;
@property (weak) IBOutlet NSTextField *enterLicenseKeyFullName;
@property (weak) IBOutlet NSTextField *enterLicenseKeyOrganization;
@property (weak) IBOutlet NSTextField *enterLicenseKeyLicenseKey;
@property (weak) IBOutlet NSButton *enterLicenseKeyCancel;
@property (weak) IBOutlet NSButton *enterLicenseKeyActivate;

@property (weak) IBOutlet NSTextField *prevVersionLicenseHintTitle;
@property (weak) IBOutlet NSTextField *prevVersionLicenseHintDescription;
@property (weak) IBOutlet NSImageView *prevVersionLicenseHintImage;
@property (weak) IBOutlet NSButton *prevVersionLicenseHintBack;
@property (weak) IBOutlet NSButton *prevVersionLicenseHintUpgrade;

@property (copy) NSNumber *numDemoDays;
@property (copy) NSNumber *numWarningDays;
@property (copy) NSNumber *numCriticalDays;

@property (copy) NSNumber *numDaysUsed;
@property (copy) NSString *daysLeftCaption;
@property (copy) NSString *demoButtonCaption;

@property (strong) IBOutlet NSWindow *enterLicenseSheet;
@property (strong) IBOutlet NSWindow *prevVersionLicenseHintSheet;

@property (copy) NSString *name;
@property (copy) NSString *organization;
@property (copy) NSString *licenseKey;

@property (copy) NSNumber *isGUICloseButtonClicked;


- (void) close: (NSInteger) code;

- (IBAction) onContinueDemoClicked: (id) sender;
- (IBAction) onPurchaseLicenseClicked: (id) sender;
- (IBAction) onEnterLicenseKeyClicked: (id) sender;
- (void) onEnterLicenseKey: (NSWindow*) window;

- (IBAction) onEnterLicenseCancelClicked: (id) sender;
- (IBAction) onEnterLicenseActivateClicked: (id) sender;

- (IBAction) onPrevVersionLicenseHintBackClicked: (id) sender;
- (IBAction) onPrevVersionLicenseHintUpgradeClicked: (id) sender;


@end

#endif

#endif // Zephyros_LicenseCheckWindowController_h
