//
//  LicenseCheckWindowController.mm
//  Ghostlab
//
//  Created by Matthias Christen on 04.03.13.
//  Copyright (c) 2013 Vanamco GmbH. All rights reserved.
//

#ifndef APPSTORE

#import "base/zephyros_impl.h"
#import "base/zephyros_strings.h"
#import "base/licensing.h"
#import "base/LicenseCheckWindowController.h"


@implementation LicenseCheckWindowController


- (id) init
{
    self.isGUICloseButtonClicked = [NSNumber numberWithBool: NO];
    
    int numDemoDays = 0;
    if (Zephyros::GetLicenseManager())
        numDemoDays = static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetNumberOfDemoDays();
    
    self.numDemoDays = [NSNumber numberWithInt: numDemoDays];
    self.numWarningDays = [NSNumber numberWithInt: numDemoDays - 2];
    self.numCriticalDays = [NSNumber numberWithInt: numDemoDays];
    
    return [self initWithWindowNibName: @"LicenseCheckWindowController"];
}

- (void) awakeFromNib
{
    self.window.title =[NSString stringWithUTF8String: Zephyros::GetString(ZS_DEMODLG_WNDTITLE).c_str()];
    self.dialogTitle.stringValue = [NSString stringWithUTF8String: Zephyros::GetString(ZS_DEMODLG_TITLE).c_str()];
    self.dialogDescription.stringValue = [NSString stringWithUTF8String: Zephyros::GetString(ZS_DEMODLG_DESCRIPTION).c_str()];
    self.remainingTimeTitle.stringValue = [NSString stringWithUTF8String: Zephyros::GetString(ZS_DEMODLG_REMAINING_TIME).c_str()];
    self.remainingTimeDescription.stringValue = [NSString stringWithUTF8String: Zephyros::GetString(ZS_DEMODLG_REMAINING_TIME_DESC).c_str()];
    self.purchaseLicense.title = [NSString stringWithUTF8String: Zephyros::GetString(ZS_DEMODLG_PURCHASE_LICENSE).c_str()];
    self.enterLicenseKey.title = [NSString stringWithUTF8String: Zephyros::GetString(ZS_DEMODLG_ENTER_LICKEY).c_str()];
    self.imageDemo.image = [NSApp applicationIconImage];
    
    self.enterLicenseSheet.title = [NSString stringWithUTF8String: Zephyros::GetString(ZS_ENTERLICKEYDLG_WNDTITLE).c_str()];
    self.enterLicenseKeyTitle.stringValue = [NSString stringWithUTF8String: Zephyros::GetString(ZS_ENTERLICKEYDLG_TITLE).c_str()];
    self.enterLicenseKeyFullName.stringValue = [NSString stringWithUTF8String: Zephyros::GetString(ZS_ENTERLICKEYDLG_FULL_NAME).c_str()];
    self.enterLicenseKeyOrganization.stringValue = [NSString stringWithUTF8String: Zephyros::GetString(ZS_ENTERLICKEYDLG_ORGANIZATION).c_str()];
    self.enterLicenseKeyLicenseKey.stringValue = [NSString stringWithUTF8String: Zephyros::GetString(ZS_ENTERLICKEYDLG_LICKEY).c_str()];
    self.enterLicenseKeyCancel.title = [NSString stringWithUTF8String: Zephyros::GetString(ZS_ENTERLICKEYDLG_CANCEL).c_str()];
    self.enterLicenseKeyActivate.title = [NSString stringWithUTF8String: Zephyros::GetString(ZS_ENTERLICKEYDLG_ACTIVATE).c_str()];
    
    self.prevVersionLicenseHintSheet.title = [NSString stringWithUTF8String: Zephyros::GetString(ZS_PREVVERSIONDLG_WNDTITLE).c_str()];
    self.prevVersionLicenseHintTitle.stringValue = [NSString stringWithUTF8String: Zephyros::GetString(ZS_PREVVERSIONDLG_TITLE).c_str()];
    self.prevVersionLicenseHintDescription.stringValue = [NSString stringWithUTF8String: Zephyros::GetString(ZS_PREVVERSIONDLG_DESCRIPTION).c_str()];
    self.prevVersionLicenseHintBack.title =[NSString stringWithUTF8String: Zephyros::GetString(ZS_PREVVERSIONDLG_BACK).c_str()];
    self.prevVersionLicenseHintUpgrade.title =[NSString stringWithUTF8String: Zephyros::GetString(ZS_PREVVERSIONDLG_UPGRADE).c_str()];
    self.prevVersionLicenseHintImage.image = [NSApp applicationIconImage];
    
    if (static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetShopURL() == NULL)
        self.purchaseLicense.hidden = YES;
    
    if (static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetUpgradeURL() == NULL)
        self.prevVersionLicenseHintUpgrade.hidden = YES;
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
    bool canContinue = numDaysUsed < static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetNumberOfDemoDays();
    if (canContinue)
        canContinue = static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->ContinueDemo();
    
    if (!canContinue || static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->HasDemoTokens())
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
    int ret = static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->Activate(
        _name == nil ? "" : [_name UTF8String],
        _organization == nil ? "" : [_organization UTF8String],
        _licenseKey == nil ? "" : [_licenseKey UTF8String]
    );
    
    switch(ret)
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
    static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->OpenUpgradeLicenseURL();
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
