//
//  LicenseCheckWindowController.h
//  Ghostlab
//
//  Created by Matthias Christen on 04.03.13.
//  Copyright (c) 2013 Vanamco GmbH. All rights reserved.
//

#ifndef APPSTORE

#import <Cocoa/Cocoa.h>
#import "base/licensing.h"


@interface LicenseCheckWindowController : NSWindowController


@property (weak) IBOutlet NSTextField *dialogTitle;
@property (weak) IBOutlet NSTextField *dialogDescription;
@property (weak) IBOutlet NSImageView *imageDemo;
@property (weak) IBOutlet NSTextField *dialogPrevVersionLicenseHintTitle;
@property (weak) IBOutlet NSTextField *dialogPrevVersionLicenseHintDescription;
@property (weak) IBOutlet NSImageView *imagePrevVersionLicenseHintSheet;

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