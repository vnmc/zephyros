/*******************************************************************************
 * Copyright (c) 2015 Vanamco AG, http://www.vanamco.com
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


#ifdef OS_WIN
#include <tchar.h>
#endif

#include "base/app.h"
#include "base/licensing.h"

#include "zephyros.h"


namespace Zephyros {

#ifndef APPSTORE

String LicenseData::Encrypt(String s)
{
    return "";
}

String LicenseData::Decrypt(String s)
{
    return "";
}
    
LicenseData::LicenseData(const TCHAR* szLicenseInfoFilename)
{
}
    
void LicenseData::Save()
{
}
    
String LicenseData::Now()
{
    return "";
}


//////////////////////////////////////////////////////////////////////////
// LicenseManager Implementation
    
LicenseManager::LicenseManager()
{
}
    
LicenseManager::~LicenseManager()
{
}
    
void LicenseManager::SetLicenseInfo(int productId, const TCHAR* szPublicKey)
{
}
    
void LicenseManager::AddObsoleteLicenseInfo(int productId, const TCHAR* szPublicKey)
{
}
    
int LicenseManager::GetNumberOfDemoDays()
{
    return 0;
}
    
void LicenseManager::SetNumberOfDemoDays(int numDemoDays)
{
}
    
void LicenseManager::SetAPIURLs(const TCHAR* demoTokensURL, const TCHAR* activationURL, const TCHAR* deactivationURL)
{
}
    
const TCHAR* LicenseManager::GetShopURL()
{
    return "";
}
    
void LicenseManager::SetShopURL(const TCHAR* shopURL)
{
}
    
const TCHAR* LicenseManager::GetUpgradeURL()
{
    return "";
}

void LicenseManager::SetUpgradeURL(const TCHAR* upgradeURL)
{
}

void LicenseManager::SetActivationLinkPrefix(const TCHAR* activationLinkPrefix)
{
}

void LicenseManager::SetLicenseInfoFilename(const TCHAR* licenseInfoFilename)
{
}

void LicenseManager::SetReceiptChecker(ReceiptChecker* pReceiptChecker)
{
}

ReceiptChecker* LicenseManager::GetReceiptChecker()
{
    return NULL;
}

void LicenseManager::Start()
{
}

bool LicenseManager::CanStartApp()
{
    return true;
}

bool LicenseManager::CheckDemoValidity()
{
    return false;
}

JavaScript::Object LicenseManager::GetLicenseInformation()
{
    return NULL;
}

int LicenseManager::ActivateFromURL(String url)
{
    return ACTIVATION_FAILED;
}

bool LicenseManager::Deactivate()
{
    return false;
}

bool LicenseManager::IsLicensingLink(const TCHAR* url)
{
    return false;
}

void LicenseManager::ShowEnterLicenseDialog()
{
}

void LicenseManager::OpenPurchaseLicenseURL()
{
}


//////////////////////////////////////////////////////////////////////////
// LicenseManagerImpl Implementation

LicenseManagerImpl::LicenseManagerImpl()
{
}

LicenseManagerImpl::~LicenseManagerImpl()
{
}

bool LicenseManagerImpl::VerifyKey(String key, String info, const TCHAR* pubkey)
{
    return false;
}

String LicenseManagerImpl::DecodeURI(String uri)
{
    return "";
}

bool LicenseManagerImpl::CheckReceipt()
{
    return false;
}

void LicenseManagerImpl::OpenPurchaseLicenseURL()
{
}

void LicenseManagerImpl::OpenUpgradeLicenseURL()
{
}

bool LicenseManagerImpl::SendRequest(String url, String postData, StringStream& out)
{
    return false;
}

void LicenseManagerImpl::ShowDemoDialog()
{
}

void LicenseManagerImpl::ShowEnterLicenseDialog()
{
}

void LicenseManagerImpl::OnActivate(bool isSuccess)
{
}

void LicenseManagerImpl::OnReceiveDemoTokens(bool isSuccess)
{
}

String LicenseManagerImpl::GetDemoButtonCaption()
{
    return "";
}

String LicenseManagerImpl::GetDaysCountLabelText()
{
    return "";
}

bool LicenseManagerImpl::ParseResponse(String url, std::string data, String errorMsg, picojson::object& result)
{
    return false;
}

int LicenseManagerImpl::Activate(String name, String company, String licenseKey)
{
    return ACTIVATION_FAILED;
}

int LicenseManagerImpl::ActivateFromURL(String url)
{
    return ACTIVATION_FAILED;
}


bool LicenseManagerImpl::Deactivate()
{
    return false;
}

void LicenseManagerImpl::Start()
{
}

bool LicenseManagerImpl::RequestDemoTokens(String strMACAddr)
{
    return false;
}

#endif
    
} // namespace Zephyros
