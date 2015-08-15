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