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


#ifndef Licensing_h
#define Licensing_h
#pragma once


#include <map>


//////////////////////////////////////////////////////////////////////////
// Constants

#define ACTIVATION_SUCCEEDED 1
#define ACTIVATION_FAILED 0
#define ACTIVATION_OBSOLETELICENSE -1


//////////////////////////////////////////////////////////////////////////
// Zephyros Classes

namespace Zephyros {

class ReceiptChecker
{
public:
    virtual void CopyAppStoreReceipt();
    String GetReceiptFilename();
    virtual String GetAppStoreBundleName() = 0;

    virtual bool CheckReceipt() = 0;
};

class AbstractLicenseManager
{
public:
    virtual ~AbstractLicenseManager() {}

    virtual void Start() {}
    virtual bool CanStartApp() = 0;
    virtual bool CheckDemoValidity() { return false; }
    virtual bool IsInDemoMode() { return false; }

    virtual std::map<String, String> GetLicenseInformation() = 0;

    virtual int ActivateFromURL(String url) { return ACTIVATION_FAILED; }
    virtual bool Deactivate() { return false; }

    inline bool IsLicensingLink(String url) { return IsLicensingLink(url.c_str()); }
    virtual bool IsLicensingLink(const TCHAR* url) { return false; }

    virtual void ShowEnterLicenseDialog() {}
    virtual void OpenPurchaseLicenseURL() {}

    virtual ReceiptChecker* GetReceiptChecker() { return NULL; }
};

class LicenseManagerImpl;

class LicenseManager : public AbstractLicenseManager
{
public:
    LicenseManager();
    virtual ~LicenseManager();

    void SetLicenseInfo(int productId, const TCHAR* szPublicKey);
    void AddObsoleteLicenseInfo(int productId, const TCHAR* szPublicKey);

    int GetNumberOfDemoDays();
    void SetNumberOfDemoDays(int numDemoDays);

    void SetAPIURLs(const TCHAR* demoTokensURL, const TCHAR* activationURL, const TCHAR* deactivationURL);

    const TCHAR* GetShopURL();
    void SetShopURL(const TCHAR* shopURL);

    const TCHAR* GetUpgradeURL();
    void SetUpgradeURL(const TCHAR* upgradeURL);

    void SetActivationLinkPrefix(const TCHAR* activationLinkPrefix);

    void SetLicenseInfoFilename(const TCHAR* licenseInfoFilename);

    void SetReceiptChecker(ReceiptChecker* pReceiptChecker);
    virtual ReceiptChecker* GetReceiptChecker();

    virtual void Start();
    virtual bool CanStartApp();
    virtual bool CheckDemoValidity();
    virtual bool IsInDemoMode();

    virtual std::map<String, String> GetLicenseInformation();

    virtual int ActivateFromURL(String url);
    virtual bool Deactivate();

    virtual bool IsLicensingLink(const TCHAR* url);

    virtual void ShowEnterLicenseDialog();
    virtual void OpenPurchaseLicenseURL();

    inline LicenseManagerImpl* GetLicenseManagerImpl() { return m_pMgr; }

private:
    LicenseManagerImpl* m_pMgr;
};

} // namespace Zephyros


#endif // Licensing_h
