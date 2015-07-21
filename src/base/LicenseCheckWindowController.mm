//
//  LicenseCheckWindowController.mm
//  Ghostlab
//
//  Created by Matthias Christen on 04.03.13.
//  Copyright (c) 2013 Vanamco GmbH. All rights reserved.
//

#ifndef APPSTORE

#import "base/zephyros_impl.h"
#import "base/licensing.h"
#import "base/LicenseCheckWindowController.h"


@implementation LicenseCheckWindowController


- (id) init
{
    self.isGUICloseButtonClicked = [NSNumber numberWithBool: NO];
    
    int numDemoDays = 0;
    if (Zephyros::GetLicenseManager())
        numDemoDays = Zephyros::GetLicenseManager()->GetNumDemoDays();
    
    self.numDemoDays = [NSNumber numberWithInt: numDemoDays];
    self.numWarningDays = [NSNumber numberWithInt: numDemoDays - 2];
    self.numCriticalDays = [NSNumber numberWithInt: numDemoDays];
    
    return [self initWithWindowNibName: @"LicenseCheckWindowController"];
}

- (void) awakeFromNib
{
    self.dialogTitle.stringValue = [NSString stringWithUTF8String: Zephyros::GetLicenseManager()->GetDialogTitle().c_str()];
    self.dialogDescription.stringValue = [NSString stringWithUTF8String: Zephyros::GetLicenseManager()->GetDialogDescription().c_str()];
    self.imageDemo.image = [NSApp applicationIconImage];
    
    self.dialogPrevVersionLicenseHintTitle.stringValue = [NSString stringWithUTF8String: Zephyros::GetLicenseManager()->GetDialogPrevLicenseHintTitle().c_str()];
    self.dialogPrevVersionLicenseHintDescription.stringValue = [NSString stringWithUTF8String: Zephyros::GetLicenseManager()->GetDialogPrevLicenseHintDescription().c_str()];
    self.imagePrevVersionLicenseHintSheet.image = [NSApp applicationIconImage];
}

- (void) windowWillClose: (NSNotification*) notification
{
    if (self.isGUICloseButtonClicked.boolValue)
    {
        // reinitialize and do nothing
        self.isGUICloseButtonClicked = [NSNumber numberWithBool: NO];
        return;
    }

    // otherwise assume that the red window button has been clicked: abort
    [NSApp stopModalWithCode: NSModalResponseAbort];
}

- (void) close: (NSInteger) code
{
    self.isGUICloseButtonClicked = [NSNumber numberWithBool: YES];
    [self.window close];
    [NSApp stopModalWithCode: code];
}

- (IBAction) onContinueDemoClicked: (id) sender
{
    int numDaysUsed = [_numDaysUsed intValue];
    bool canContinue = numDaysUsed < Zephyros::GetLicenseManager()->GetNumDemoDays();
    if (canContinue)
        canContinue = Zephyros::GetLicenseManager()->ContinueDemo();
    
    if (!canContinue || Zephyros::GetLicenseManager()->HasDemoTokens())
        [self close: canContinue ? NSModalResponseContinue : NSModalResponseAbort];
}

- (IBAction) onPurchaseLicenseClicked: (id) sender
{
    Zephyros::GetLicenseManager()->OpenPurchaseLicenseURL();
}

- (IBAction) onEnterLicenseKeyClicked: (id) sender
{
    [self onEnterLicenseKey: self.window];
}

- (void) onEnterLicenseKey: (NSWindow*) window
{
    [NSApp beginSheet: _enterLicenseSheet
       modalForWindow: window
        modalDelegate: self
       didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
          contextInfo: nil];
}

- (IBAction) onEnterLicenseCancelClicked: (id) sender
{
    [NSApp endSheet: _enterLicenseSheet];
}

- (IBAction) onEnterLicenseActivateClicked: (id) sender
{
    switch(Zephyros::GetLicenseManager()->Activate(
        _name == nil ? "" : [_name UTF8String],
        _organization == nil ? "" : [_organization UTF8String],
        _licenseKey == nil ? "" : [_licenseKey UTF8String]
    ))
    {
    case ACTIVATION_SUCCEEDED:
        [NSApp endSheet: _enterLicenseSheet];
        break;
            
    case ACTIVATION_OBSOLETELICENSE:
        [NSApp endSheet: _enterLicenseSheet];
        [NSApp beginSheet: _prevVersionLicenseHintSheet
           modalForWindow: self.window
            modalDelegate: self
           didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
              contextInfo: nil];
        break;
    }
}

- (IBAction) onPrevVersionLicenseHintBackClicked: (id) sender
{
    [NSApp endSheet: _prevVersionLicenseHintSheet];
}

- (IBAction) onPrevVersionLicenseHintUpgradeClicked: (id) sender
{
    Zephyros::GetLicenseManager()->OpenUpgradeLicenseURL();
}

/**
 * General method to end modal sheets.
 * Reference this method as didEndSelector when displaying sheets.
 */
- (void) didEndSheet: (NSWindow*) sheet returnCode: (int) returnCode contextInfo: (void*) contextInfo
{
    [sheet orderOut: self];
}


@end

#endif
