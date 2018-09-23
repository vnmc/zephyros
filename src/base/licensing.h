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
 * Florian Müller, Vanamco AG
 *******************************************************************************/


#ifndef Zephyros_licensing_h
#define Zephyros_licensing_h
#pragma once

#include <vector>

#ifdef OS_MACOSX
#ifdef __OBJC__
#include <Foundation/Foundation.h>
#else
#include <objc/objc.h>
#endif
#endif

#ifdef OS_WIN
#include <tchar.h>
#endif

#include "zephyros.h"
#include "base/types.h"
#include "base/logging.h"

#include "util/base32.h"
#include "util/picojson.h"

#include "native_extensions/network_util.h"


//////////////////////////////////////////////////////////////////////////
// Type Definitions

namespace Zephyros {

class AbstractLicenseManager;
class LicenseManagerImpl;
class ReceiptChecker;


typedef struct
{
    int productId;
    const TCHAR* pubkey;
} LicenseInfo;

typedef struct
{
    LicenseInfo currentLicenseInfo;
    std::vector<LicenseInfo> prevLicenseInfo;

    int numDemoDays;

    const TCHAR* demoTokensURL;
    const TCHAR* activationURL;
    const TCHAR* deactivationURL;

    const TCHAR* shopURL;
    const TCHAR* upgradeURL;

    const TCHAR* activationLinkPrefix;

    const TCHAR* licenseInfoFilename;

    ReceiptChecker* pReceiptChecker;
} LicenseManagerConfig;

} // namespace Zephyros


//////////////////////////////////////////////////////////////////////////
// Classes

namespace Zephyros {

class LicenseData
{
    friend class LicenseManagerImpl;

public:
    LicenseData(const TCHAR* szLicenseInfoFilename);
    inline bool GetDemoToken()
    {
        if (m_demoTokens.size() == 0)
            return false;

        String hash = LicenseData::Now();
        if (m_timestampLastDemoTokenUsed == TEXT("") || m_timestampLastDemoTokenUsed != hash)
        {
            m_demoTokens.erase(m_demoTokens.begin());
            m_timestampLastDemoTokenUsed = hash;
            Save();
        }

        return true;
    }

    static String Now();

private:
    void Save();

    static String Encrypt(String s);
    static String Decrypt(String s);

    const TCHAR* m_szLicenseInfoFilename;

    std::vector<String> m_demoTokens;
    String m_timestampLastDemoTokenUsed;
    String m_activationCookie;
    String m_licenseKey;
    String m_name;
    String m_company;

    static char gheim[129];
};

} // namespace Zephyros


//#if APPSTORE == 0 || !defined(APPSTORE)
#ifdef OS_MACOSX
#if __OBJC__

@class LicenseCheckWindowController;

@interface LicenseManagerTimerDelegate : NSObject
{
@public
    Zephyros::LicenseManagerImpl* m_pLicenseManager;
}

- (void) onTimeout: (NSTimer*) timer;
- (void) onSysTimeChanged: (NSNotification*) notification;

@end

typedef LicenseCheckWindowController* LicenseCheckWindowControllerRef;
typedef LicenseManagerTimerDelegate* LicenseManagerTimerDelegateRef;

#else

typedef id LicenseCheckWindowControllerRef;
typedef id LicenseManagerTimerDelegateRef;

#endif // __OBJC__
#endif // OS_MACOSX


namespace Zephyros {

#ifdef OS_WIN
    class DemoDialog;
#endif

class LicenseManagerImpl : public AbstractLicenseManager
{
public:
    LicenseManagerImpl();
    virtual ~LicenseManagerImpl();

    inline void SetLicenseInfo(int productId, const TCHAR* szPublicKey)
    {
        m_config.currentLicenseInfo.productId = productId;
        m_config.currentLicenseInfo.pubkey = szPublicKey;
    }

    inline void AddObsoleteLicenseInfo(int productId, const TCHAR* szPublicKey)
    {
         LicenseInfo info;
        info.productId = productId;
        info.pubkey = szPublicKey;

        m_config.prevLicenseInfo.push_back(info);
    }

    inline int GetNumberOfDemoDays() { return m_config.numDemoDays; }
    inline void SetNumberOfDemoDays(int numDemoDays) { m_config.numDemoDays = numDemoDays; }

    inline void SetAPIURLs(const TCHAR* demoTokensURL, const TCHAR* activationURL, const TCHAR* deactivationURL)
    {
        m_config.demoTokensURL = demoTokensURL;
        m_config.activationURL = activationURL;
        m_config.deactivationURL = deactivationURL;
    }

    inline const TCHAR* GetShopURL() { return m_config.shopURL; }
    inline void SetShopURL(const TCHAR* shopURL) { m_config.shopURL = shopURL; }

    inline const TCHAR* GetUpgradeURL() { return m_config.upgradeURL; }
    inline void SetUpgradeURL(const TCHAR* upgradeURL) { m_config.upgradeURL = upgradeURL; }

    inline void SetActivationLinkPrefix(const TCHAR* activationLinkPrefix) { m_config.activationLinkPrefix = activationLinkPrefix; }

    inline void SetLicenseInfoFilename(const TCHAR* licenseInfoFilename) { m_config.licenseInfoFilename = licenseInfoFilename; }

    inline ReceiptChecker* GetReceiptChecker() { return m_config.pReceiptChecker; }
    inline void SetReceiptChecker(ReceiptChecker* pReceiptChecker) { m_config.pReceiptChecker = pReceiptChecker; }
    
    void LoadLicenseData();

    /**
     * Starts the license manager.
     */
    virtual void Start();

    bool RequestDemoTokens(String strMACAddr = TEXT(""));
    
    void FireLicenseChanged();

    String DecodeURI(String uri);

    int Activate(String name, String company, String licenseKey);
    virtual int ActivateFromURL(String url);
    virtual bool Deactivate();

    virtual bool IsLicensingLink(const TCHAR* url)
    {
        if (!m_config.activationLinkPrefix || !url)
            return false;

        size_t len = _tcslen(m_config.activationLinkPrefix);
        return _tcsnccmp(url, m_config.activationLinkPrefix, len) == 0;
    }

    /**
     * Invoke this when the "Continue Demo" button was clicked on the demo dialog.
     * Returns true if the demo can be continued. If the method returns false, the
     * application should quit.
     */
    inline bool ContinueDemo()
    {
        // has the demo been started yet? if not, request demo tokens
        if (m_pLicenseData->m_demoTokens.size() == 0 && m_pLicenseData->m_timestampLastDemoTokenUsed == TEXT(""))
            return m_canStartApp = RequestDemoTokens();

        if (!m_pLicenseData->GetDemoToken() || !VerifyAll(m_pLicenseData->m_demoTokens.at(0), NetworkUtil::GetAllMACAddresses(), m_config.currentLicenseInfo.pubkey))
        {
            // invalid token; destroy demo tokens
            m_pLicenseData->m_demoTokens.clear();
            m_pLicenseData->Save();
            m_canStartApp = false;
            return false;
        }

        return true;
    }

    /**
     * Check whether the demo is still valid.
     * If the product hasn't been activated, the demo dialog is shown.
     * Returns true iff the product has been activated.
     */
    inline virtual bool CheckDemoValidity() final
    {
#ifdef OS_WIN
        // prevent showing too many demo dialogs when WM_TIMECHANGE is sent multiple times
        ULONGLONG tick = GetTickCount64();
        if (m_lastDemoValidityCheck > 0 && tick - m_lastDemoValidityCheck < (6 * 3600 * 1000))
            return true;
        m_lastDemoValidityCheck = tick;
#endif

        // nothing to do if the app has been activated
        if (IsActivated())
            return true;

        // consume a token if it isn't the same day
        m_canStartApp = false;
        
        size_t oldNumDemoTokens = m_pLicenseData->m_demoTokens.size();
        m_pLicenseData->GetDemoToken();
        
        if (oldNumDemoTokens != m_pLicenseData->m_demoTokens.size())
            FireLicenseChanged();
        
        ShowDemoDialog();
        return false;
    }

    virtual void ShowEnterLicenseDialog();

#ifdef OS_LINUX
    void ShowUpgradeLicenseDialog();
#endif

    virtual void OpenPurchaseLicenseURL();
    void OpenUpgradeLicenseURL();

    inline virtual bool CanStartApp() final { return m_canStartApp; }

    /**
     * Tests whether the product has been activated.
     * Returns true if the product was activated.
     */
    inline bool IsActivated()
    {
        bool ret =
            m_pLicenseData->m_activationCookie.length() > 0 &&
            VerifyAll(m_pLicenseData->m_activationCookie, NetworkUtil::GetAllMACAddresses(), m_config.currentLicenseInfo.pubkey);
        
#ifdef OS_MACOSX
        // on Mac, if the activation check wasn't successful, also check whether there
        // is a AppStore receipt, and validate it
        if (!ret)
            ret = CheckReceipt();
#endif

        // try to activate with stored license
        if (!ret && m_pLicenseData->m_licenseKey != TEXT(""))
            ret = Activate(m_pLicenseData->m_name, m_pLicenseData->m_company, m_pLicenseData->m_licenseKey) == ACTIVATION_SUCCEEDED;

        return ret;
    }

    inline bool HasDemoTokens() { return m_pLicenseData->m_demoTokens.size() > 0; }

    inline virtual void GetLicenseInformation(void* ret) final
    {
        JavaScript::Object info = *(static_cast<JavaScript::Object*>(ret));

        bool isActivated = IsActivated();
        int numDemoDays = GetNumberOfDemoDays();
        int numDaysLeft = GetNumDaysLeft();

        info->SetString(TEXT("mac"), NetworkUtil::GetPrimaryMACAddress());
        info->SetString(TEXT("licenseKey"), m_pLicenseData ? m_pLicenseData->m_licenseKey : TEXT(""));
        info->SetString(TEXT("fullName"), m_pLicenseData ? m_pLicenseData->m_name : TEXT(""));
        info->SetString(TEXT("company"), m_pLicenseData ? m_pLicenseData->m_company : TEXT(""));
        info->SetBool(TEXT("isActivated"), isActivated);
        info->SetInt(TEXT("demoDaysUsed"), m_pLicenseData ? numDemoDays - numDaysLeft : numDemoDays);
        info->SetInt(TEXT("demoDaysLeft"), m_pLicenseData ? numDaysLeft : 0);

        if (isActivated)
            info->SetString(TEXT("token"), m_pLicenseData->m_activationCookie);
        else
        {
            if (m_pLicenseData && m_pLicenseData->m_demoTokens.size() > 0)
                info->SetString(TEXT("token"), m_pLicenseData->m_demoTokens[0]);
            else
                info->SetString(TEXT("token"), TEXT(""));
        }
    }

    bool CheckReceipt();

    String GetDemoButtonCaption();
    String GetDaysCountLabelText();

private:
    inline void InitConfig()
    {
        m_pLicenseData = NULL;

        m_config.currentLicenseInfo.productId = 0;
        m_config.currentLicenseInfo.pubkey = NULL;

        m_config.numDemoDays = 7;

        m_config.demoTokensURL = NULL;
        m_config.activationURL = NULL;
        m_config.deactivationURL = NULL;

        m_config.shopURL = NULL;
        m_config.upgradeURL = NULL;

        m_config.activationLinkPrefix = NULL;

        m_config.licenseInfoFilename = TEXT("lic.dat");

        m_config.pReceiptChecker = NULL;
    }

    inline int GetNumDaysLeft()
    {
        int numDaysLeft = (int) m_pLicenseData->m_demoTokens.size();
        if (m_pLicenseData->m_timestampLastDemoTokenUsed == TEXT("") || m_pLicenseData->m_timestampLastDemoTokenUsed != LicenseData::Now())
            numDaysLeft--;
        if (numDaysLeft < 0)
            numDaysLeft = m_pLicenseData->m_timestampLastDemoTokenUsed == TEXT("") ? m_config.numDemoDays : 0;

        return numDaysLeft;
    }

    void ShowDemoDialog();

    bool SendRequest(String url, std::string data, std::stringstream& out);
    bool ParseResponse(String url, std::string data, String errorMsg, picojson::object& result);

    void OnActivate(bool isSuccess);
    void OnReceiveDemoTokens(bool isSuccess);

    bool VerifyKey(String key, String info, const TCHAR* pubkey);

    bool VerifyAll(String key, std::vector<String> v, const TCHAR* pubkey)
    {
        for (String k : v)
            if (VerifyKey(key, k, pubkey))
                return true;
        return false;
    }

    inline bool DecodeKey(String key, unsigned char** pRetBuf, size_t* pRetBufLen)
    {
        size_t rawKeyLen = 0;
        for (String::iterator it = key.begin(); it != key.end(); ++it)
            if (*it != TEXT('-'))
                rawKeyLen++;

        // pad to octets
        rawKeyLen = ((rawKeyLen + 7) / 8) * 8;

        uint8_t* pRawKey = new uint8_t[rawKeyLen];
        int idx = 0;
        for (String::iterator it = key.begin(); it != key.end(); ++it)
        {
            // replace 9s with Is and 8s with Os
            if (*it == TEXT('9'))
                pRawKey[idx++] = 'I';
            else if (*it == TEXT('8'))
                pRawKey[idx++] = 'O';
            else if ((TEXT('A') <= *it && *it <= TEXT('Z')) || (TEXT('0') <= *it && *it <= TEXT('9')))
                pRawKey[idx++] = (uint8_t) *it;
        }

        // pad with '='
        while (idx < (int) rawKeyLen)
            pRawKey[idx++] = '=';

        // decode the base32 string
        *pRetBufLen = base32_decoder_buffer_size(rawKeyLen);
        *pRetBuf = new unsigned char[*pRetBufLen];
        if (*pRetBuf == NULL)
            return false;

        *pRetBufLen = base32_decode(*pRetBuf, *pRetBufLen, pRawKey, rawKeyLen);
        return *pRetBufLen > 0;
    }

protected:
    LicenseManagerConfig m_config;

private:
    LicenseData* m_pLicenseData;
    bool m_canStartApp;


#ifdef OS_MACOSX
public:
    LicenseCheckWindowControllerRef m_windowController;
private:
    LicenseManagerTimerDelegateRef m_timerDelegate;
#endif

#ifdef OS_WIN
public:
    void KillTimer();

    DemoDialog* m_pDemoDlg;

private:
    UINT_PTR m_timerId;
    ULONGLONG m_lastDemoValidityCheck;
#endif

#ifdef OS_LINUX
public:
    GtkWidget* m_pDemoDlg;
#endif
};

//#endif  // APPSTORE == 0

} // namespace Zephyros


#endif // Zephyros_licensing_h
