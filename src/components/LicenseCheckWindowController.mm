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


#ifndef APPSTORE

#import "zephyros_strings.h"
#import "base/licensing.h"
#import "components/LicenseCheckWindowController.h"


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
    
    const TCHAR* szShopURL = static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetShopURL();
    if (szShopURL == NULL || szShopURL[0] == TCHAR('\0'))
        self.purchaseLicense.hidden = YES;
    
    const TCHAR* szUpgradeURL = static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetUpgradeURL();
    if (szUpgradeURL == NULL || szUpgradeURL[0] == TCHAR('\0'))
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
        canContinue = static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetLicenseManagerImpl()->ContinueDemo();
    
    if (!canContinue || static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetLicenseManagerImpl()->HasDemoTokens())
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
    int ret = static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetLicenseManagerImpl()->Activate(
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
    static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetLicenseManagerImpl()->OpenUpgradeLicenseURL();
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

#endif // !APPSTORE
