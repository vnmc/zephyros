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


#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>

#import <Security/Security.h>
#import <CommonCrypto/CommonDigest.h>

#import <iomanip>

#import "base/app.h"
#import "base/licensing.h"

#import "components/LicenseCheckWindowController.h"

#import "native_extensions/network_util.h"


@implementation LicenseManagerTimerDelegate

- (void) onTimeout: (NSTimer*) timer
{
    m_pLicenseManager->CheckDemoValidity();
}

- (void) onSysTimeChanged: (NSNotification*) notification
{
    m_pLicenseManager->CheckDemoValidity();
}

@end


namespace Zephyros {

#ifndef APPSTORE


//////////////////////////////////////////////////////////////////////////
// LicenseData Implementation

LicenseData::LicenseData(const TCHAR* szLicenseInfoFilename)
    : m_szLicenseInfoFilename(szLicenseInfoFilename)
{
    NSArray *pathList = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *dataFilePath = [[pathList objectAtIndex: 0] stringByAppendingPathComponent: [[NSBundle mainBundle] bundleIdentifier]];
    NSFileManager *fileManager= [NSFileManager defaultManager];
    
    BOOL isDir;
    if (![fileManager fileExistsAtPath: dataFilePath isDirectory: &isDir])
        [fileManager createDirectoryAtPath: dataFilePath withIntermediateDirectories: YES attributes: nil error: NULL];
    
    NSString *licenseFileName = [NSString stringWithUTF8String: szLicenseInfoFilename];
    
    NSString *licenseFilePath = [dataFilePath stringByAppendingPathComponent: licenseFileName];
    
    if (![fileManager fileExistsAtPath:licenseFilePath])
    {
        pathList = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSSystemDomainMask, YES);
        dataFilePath = [[pathList objectAtIndex: 0] stringByAppendingPathComponent: [[NSBundle mainBundle] bundleIdentifier]];
        licenseFilePath = [dataFilePath stringByAppendingPathComponent: licenseFileName];
    }
    
    NSDictionary *root = [NSKeyedUnarchiver unarchiveObjectWithFile: licenseFilePath];

    NSArray *arrDemoTokens = nil;
    NSString *timestamp = nil;
    NSString *activationCookie = nil;
    NSString *licenseKey = nil;
    NSString *name = nil;
    NSString *company = nil;
    
    if (root != nil)
    {
        arrDemoTokens = [root valueForKey: @"dtoks"];
        timestamp = [root valueForKey: @"tstmp"];
        activationCookie = [root valueForKey: @"ac"];
        licenseKey = [root valueForKey: @"lk"];
        name = [root valueForKey: @"n"];
        company = [root valueForKey: @"c"];
    }
    
    if (arrDemoTokens != nil)
        for (NSString *token in arrDemoTokens)
            m_demoTokens.push_back(String([token UTF8String]));
    
    m_timestampLastDemoTokenUsed = timestamp == nil ? "" : String([timestamp UTF8String]);
    m_activationCookie = activationCookie == nil ? "" : String([activationCookie UTF8String]);
    m_licenseKey = licenseKey == nil ? "" : LicenseData::Decrypt(String([licenseKey UTF8String]));
    m_name = name == nil ? "" : LicenseData::Decrypt(String([name UTF8String]));
    m_company = company == nil ? "" : LicenseData::Decrypt(String([company UTF8String]));
}

void LicenseData::Save()
{
    NSArray *pathList = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *dataFilePath = [[pathList objectAtIndex: 0] stringByAppendingPathComponent: [[NSBundle mainBundle] bundleIdentifier]];
    dataFilePath = [dataFilePath stringByAppendingPathComponent: [NSString stringWithUTF8String: m_szLicenseInfoFilename]];

    NSMutableDictionary *root = [[NSMutableDictionary alloc] init];
    
    NSMutableArray *arrDemoTokens = [[NSMutableArray alloc] init];
    for (String token : m_demoTokens)
        [arrDemoTokens addObject: [NSString stringWithUTF8String: token.c_str()]];
    [root setValue: arrDemoTokens forKey: @"dtoks"];
    
    [root setValue: [NSString stringWithUTF8String: m_timestampLastDemoTokenUsed.c_str()] forKey: @"tstmp"];
    [root setValue: [NSString stringWithUTF8String: m_activationCookie.c_str()] forKey: @"ac"];
    [root setValue: [NSString stringWithUTF8String: LicenseData::Encrypt(m_licenseKey).c_str()] forKey: @"lk"];
    [root setValue: [NSString stringWithUTF8String: LicenseData::Encrypt(m_name).c_str()] forKey: @"n"];
    [root setValue: [NSString stringWithUTF8String: LicenseData::Encrypt(m_company).c_str()] forKey: @"c"];
    
    [NSKeyedArchiver archiveRootObject: root toFile: dataFilePath];
}

String LicenseData::Now()
{
    NSDateComponents *now = [[NSCalendar currentCalendar] components: NSCalendarUnitDay | NSCalendarUnitMonth | NSCalendarUnitYear fromDate: [NSDate date]];
    NSString *str = [NSString stringWithFormat: @"%ld-%ld-%ld-%ld", (long) now.year, (long) now.month, (long) now.day, (long) now.weekday];
    NSData *data = [str dataUsingEncoding: NSUTF8StringEncoding];
    
    // compute the SHA1 hash of input.
    // http://www.makebetterthings.com/iphone/how-to-get-md5-and-sha1-in-objective-c-ios-sdk/
    uint8_t digest[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1(data.bytes, (int) data.length, digest);
    
    StringStream ssResult;
    ssResult << std::hex << std::setfill(TEXT('0'));
    for (int i = 0; i < (int) CC_SHA1_DIGEST_LENGTH; ++i)
        ssResult << std::setw(2) << (int) digest[i];
    
    return ssResult.str();
}


//////////////////////////////////////////////////////////////////////////
// LicenseManager Implementation

LicenseManagerImpl::LicenseManagerImpl()
    : m_windowController(nil)
{
    InitConfig();
    
    m_timerDelegate = [[LicenseManagerTimerDelegate alloc] init];
    m_timerDelegate->m_pLicenseManager = this;
    
    [[NSNotificationCenter defaultCenter] addObserver: m_timerDelegate
                                             selector: @selector(onSysTimeChanged:)
                                                 name: NSSystemClockDidChangeNotification
                                               object: nil];
}

LicenseManagerImpl::~LicenseManagerImpl()
{
    if (m_pLicenseData != NULL)
        delete m_pLicenseData;
    
    [[NSNotificationCenter defaultCenter] removeObserver: m_timerDelegate name: NSSystemClockDidChangeNotification object: nil];
}

//
// Cf. https://gist.github.com/mattstevens/3493552
//
bool LicenseManagerImpl::VerifyKey(String key, String info, const TCHAR* pubkey)
{
    bool result = false;

    String strPubKey = "-----BEGIN DSA PUBLIC KEY-----\n";
    strPubKey.append(pubkey);
    strPubKey.append("\n-----END DSA PUBLIC KEY-----\n");
    
    NSData *publicKeyData = [NSData dataWithBytes: strPubKey.c_str() length: strPubKey.length()];
    SecItemImportExportKeyParameters params = {};
    SecExternalItemType keyType = kSecItemTypePublicKey;
    SecExternalFormat keyFormat = kSecFormatPEMSequence;
    CFArrayRef importArray = NULL;
    
    SecItemImport((__bridge CFDataRef) publicKeyData, NULL, &keyFormat, &keyType, 0, &params, NULL, &importArray);
    SecKeyRef publicKey = (SecKeyRef) CFArrayGetValueAtIndex(importArray, 0);
    
    unsigned char* buf = NULL;
    size_t len = 0;
    if (DecodeKey(key, &buf, &len))
    {
        CFDataRef signature = CFDataCreate(kCFAllocatorDefault, buf, len);
        NSData *digest = [NSData dataWithBytes: info.c_str() length: info.length()];
    
        SecTransformRef verifier = SecVerifyTransformCreate(publicKey, signature, NULL);
        SecTransformSetAttribute(verifier, kSecTransformInputAttributeName, (__bridge CFTypeRef) digest, NULL);
        SecTransformSetAttribute(verifier, kSecDigestTypeAttribute, kSecDigestSHA1, NULL);
    
        CFErrorRef error;
        CFBooleanRef transformResult = (CFBooleanRef) SecTransformExecute(verifier, &error);
        if (transformResult)
        {
            result = (transformResult == kCFBooleanTrue);
            CFRelease(transformResult);
        }
    
        CFRelease(signature);
        CFRelease(verifier);
        delete[] buf;
    }
    
    CFRelease(importArray);
    
    return result;
}

String LicenseManagerImpl::DecodeURI(String uri)
{
    return String([[[[NSString stringWithUTF8String: uri.c_str()] stringByReplacingOccurrencesOfString: @"+" withString: @" "] stringByReplacingPercentEscapesUsingEncoding: NSUTF8StringEncoding] UTF8String]);
}

bool LicenseManagerImpl::CheckReceipt()
{
    if (!m_config.pReceiptChecker)
        return false;
    
    // copy the receipt to check to the standard location
    NSBundle* bundle = [NSBundle mainBundle];
    NSFileManager* fileManager= [NSFileManager defaultManager];
    
    NSString* masBundleIdentifier = [NSString stringWithUTF8String: m_config.pReceiptChecker->GetAppStoreBundleName().c_str()];
    String receiptFilename = m_config.pReceiptChecker->GetReceiptFilename();
    
    // the receipt is saved in the sandbox app container at
    // ~/Library/Containers/<bundle-id>/Data/Library/Application Support/<bundle-id>/MASReceipts/<mac-addr>
    NSArray* pathList = [fileManager URLsForDirectory: NSApplicationSupportDirectory inDomains: NSUserDomainMask];
    NSArray* pathComponents = [[pathList objectAtIndex: 0] pathComponents];
    NSString* dataFilePath = [[[[[[[[[[[pathList objectAtIndex: 0] path] stringByDeletingLastPathComponent]
        stringByAppendingPathComponent: @"Containers"]
        stringByAppendingPathComponent: masBundleIdentifier]
        stringByAppendingPathComponent: @"Data"]
        stringByAppendingPathComponent: [pathComponents objectAtIndex: pathComponents.count - 2]]
        stringByAppendingPathComponent: [pathComponents objectAtIndex: pathComponents.count - 1]]
        stringByAppendingPathComponent: masBundleIdentifier]
        stringByAppendingPathComponent: @"MASReceipts"]
        stringByAppendingPathComponent: [NSString stringWithUTF8String: receiptFilename.c_str()]];
    
    if ([fileManager fileExistsAtPath: dataFilePath])
    {
        NSData* data = [NSData dataWithContentsOfFile: dataFilePath];
        uint8_t* bytes = (uint8_t*) data.bytes;
        int idx = 0;
        
        // 1. length of the receipt
        int64_t len = *((int64_t*) (bytes));
        idx += sizeof(int64_t);
        
        // 2. receipt data
        NSData* receiptData = [NSData dataWithBytes: bytes + idx length: len];
        idx += len;
        
        // 3. MAC address, must match the file name
        String macAddr(bytes + idx, bytes + idx + receiptFilename.length());
        if (macAddr != receiptFilename)
            return false;
        idx += receiptFilename.length();
        
        // 4. SHA of 1..3
        uint8_t digestCalc[CC_SHA1_DIGEST_LENGTH];
        CC_SHA1(bytes, idx, digestCalc);
        for (int i = 0; i < CC_SHA1_DIGEST_LENGTH; ++i)
        {
            if (bytes[idx + i] != digestCalc[i])
            {
                //NSLog(@"%d: %d != %d", i, bytes[idx + i], digestCalc[i]);
                return false;
            }
        }

        // copy the receipt to a standard location
        if ([receiptData writeToFile: [[[[pathList objectAtIndex: 0] path]
            stringByAppendingPathComponent: [bundle bundleIdentifier]]
            stringByAppendingPathComponent: @"rvl"]
                          atomically: NO])
        {
            return m_config.pReceiptChecker->CheckReceipt();
        }
    }
    
    return false;
}

void LicenseManagerImpl::OpenPurchaseLicenseURL()
{
    if (m_config.shopURL)
        [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: [NSString stringWithUTF8String: m_config.shopURL]]];
}

void LicenseManagerImpl::OpenUpgradeLicenseURL()
{
    if (m_config.upgradeURL)
        [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: [NSString stringWithUTF8String: m_config.upgradeURL]]];
}

bool LicenseManagerImpl::SendRequest(String url, String postData, StringStream& out)
{
    NSMutableURLRequest *urlRequest = [NSMutableURLRequest requestWithURL: [NSURL URLWithString: [NSString stringWithUTF8String: url.c_str()]]];
    urlRequest.HTTPMethod = @"POST";
    urlRequest.HTTPBody = [[NSString stringWithUTF8String: postData.c_str()] dataUsingEncoding: NSUTF8StringEncoding];
    [urlRequest setValue: @"application/x-www-form-urlencoded" forHTTPHeaderField: @"Content-Type"];
    [urlRequest setValue: [NSString stringWithFormat: @"%ld", urlRequest.HTTPBody.length] forHTTPHeaderField: @"Content-Length"];
    
    NSURLResponse *response = nil;
    NSError *error = nil;
    NSData *data = [NSURLConnection sendSynchronousRequest: urlRequest
                                         returningResponse: &response
                                                     error: &error];
    if (error == nil)
    {
        NSString *ret = [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding];
        out << [ret UTF8String];
    }

    return error == nil;
}

void LicenseManagerImpl::ShowDemoDialog()
{
    if (m_windowController == nil)
        m_windowController = [[LicenseCheckWindowController alloc] init];
    
    m_windowController.numDaysUsed = [NSNumber numberWithInt: m_config.numDemoDays - GetNumDaysLeft()];
    m_windowController.daysLeftCaption = [NSString stringWithUTF8String: GetDaysCountLabelText().c_str()];
    m_windowController.demoButtonCaption = [NSString stringWithUTF8String: GetDemoButtonCaption().c_str()];
    
    [[m_windowController window] center];
    
    NSInteger result = [NSApp runModalForWindow: [m_windowController window]];
    if (result == NSModalResponseContinue)
        m_canStartApp = true;
    
    // install a timer to do regular checks
    if (m_canStartApp && !IsActivated())
        [NSTimer scheduledTimerWithTimeInterval: 6 * 3600 target: m_timerDelegate selector: @selector(onTimeout:) userInfo: nil repeats: NO];
    
    // terminate if the app can't be started and the main window is open
    if (!m_canStartApp)
        Zephyros::App::QuitMessageLoop();
}

void LicenseManagerImpl::ShowEnterLicenseDialog()
{
    if (m_windowController == nil)
        m_windowController = [[LicenseCheckWindowController alloc] init];
    
    [m_windowController onEnterLicenseKey: [NSApp mainWindow]];
}


void LicenseManagerImpl::OnActivate(bool isSuccess)
{
    if (m_windowController != nil)
        [m_windowController close: isSuccess ? NSModalResponseContinue : NSModalResponseAbort];
}

void LicenseManagerImpl::OnReceiveDemoTokens(bool isSuccess)
{
    if (m_windowController != nil)
        [m_windowController close: isSuccess ? NSModalResponseContinue : NSModalResponseAbort];
}

#endif // !APPSTORE


String ReceiptChecker::GetReceiptFilename()
{
    String strUID = NetworkUtil::GetPrimaryMACAddress();
        
    StringStream ssUID;
    for (char c : strUID)
        if (c != ':')
            ssUID << c;
        
    return ssUID.str();
}

void ReceiptChecker::CopyAppStoreReceipt()
{
    NSBundle* bundle = [NSBundle mainBundle];
    NSFileManager* fileManager= [NSFileManager defaultManager];
    
    NSArray* pathList = [fileManager URLsForDirectory: NSApplicationSupportDirectory inDomains: NSUserDomainMask];
    NSString* dataFilePath = [[[pathList objectAtIndex: 0] path] stringByAppendingPathComponent: [bundle bundleIdentifier]];

    BOOL isDir;
    if (![fileManager fileExistsAtPath: dataFilePath isDirectory: &isDir])
        [fileManager createDirectoryAtPath: dataFilePath withIntermediateDirectories: YES attributes: nil error: NULL];

    dataFilePath = [dataFilePath stringByAppendingPathComponent: @"MASReceipts"];
    if (![fileManager fileExistsAtPath: dataFilePath isDirectory: &isDir])
        [fileManager createDirectoryAtPath: dataFilePath withIntermediateDirectories: YES attributes: nil error: NULL];
    
    const char* szFilename = GetReceiptFilename().c_str();

    NSData* receiptData = [NSData dataWithContentsOfURL: [bundle appStoreReceiptURL]];
    if (receiptData != nil)
    {
        NSMutableData* data = [[NSMutableData alloc] init];
        
        // 1. length of the receipt
        int64_t len = (int64_t) receiptData.length;
        [data appendBytes: &len length: sizeof(int64_t)];
        
        // 2. receipt data
        [data appendData: receiptData];
        
        // 3. MAC address (without colons)
        [data appendBytes: szFilename length: strlen(szFilename)];

        // 4. SHA1 of 1..3
        uint8_t digest[CC_SHA1_DIGEST_LENGTH];
        CC_SHA1(data.bytes, (int) data.length, digest);
        [data appendBytes: digest length: CC_SHA1_DIGEST_LENGTH];

        [data writeToFile: [NSString stringWithFormat: @"%@/%s", dataFilePath, szFilename] atomically: NO];
    }
}


} // namespace Zephyros
