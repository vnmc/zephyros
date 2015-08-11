#ifdef OS_WIN
#include <tchar.h>
#endif

#include "base/zephyros_impl.h"
#include "base/app.h"
#include "base/licensing.h"


namespace Zephyros {

#ifndef APPSTORE

String LicenseData::Encrype(String s)
{
    return TEXT("");
}

String LicenseData::Decrype(String s)
{
    return TEXT("");
}
    
LicenseData::LicenseData()
{
}
    
void LicenseData::Save()
{
}
    
String LicenseData::Now()
{
    return TEXT("");
}


//////////////////////////////////////////////////////////////////////////
// LicenseManager Implementation
    
LicenseManager::LicenseManager(LicenseManagerInfo* info)
{
}
    
LicenseManager::~LicenseManager()
{
}
    
bool LicenseManager::VerifyKey(String key, String info, const TCHAR* pubkey)
{
    return false;
}
    
String LicenseManager::DecodeURI(String uri)
{
    return TEXT("");
}
    
bool LicenseManager::CheckReceipt()
{
    return false;
}
    
void LicenseManager::OpenPurchaseLicenseURL()
{
}
    
void LicenseManager::OpenUpgradeLicenseURL()
{
}
    
bool LicenseManager::SendRequest(String urlPath, String postData, StringStream& out)
{
    return false;
}
    
void LicenseManager::ShowDemoDialog()
{
}
    
void LicenseManager::ShowEnterLicenseDialog()
{
}
    
void LicenseManager::OnActivate(bool isSuccess)
{
}
    
void LicenseManager::OnReceiveDemoTokens(bool isSuccess)
{
}
    
String ReceiptChecker::GetReceiptFilename()
{
    return TEXT("");
}
    
void ReceiptChecker::CopyAppStoreReceipt()
{
}
    
String LicenseManager::GetDialogTitle()
{
    return TEXT("");
}
    
String LicenseManager::GetDialogDescription()
{
    return TEXT("");
}
    
String LicenseManager::GetDialogPrevLicenseHintTitle()
{
    return TEXT("");
}
    
String LicenseManager::GetDialogPrevLicenseHintDescription()
{
    return TEXT("");
}

String LicenseManager::GetDemoButtonCaption()
{
    return TEXT("");
}

String LicenseManager::GetDaysCountLabelText()
{
    return TEXT("");
}

bool LicenseManager::ParseResponse(String url, std::string data, String errorMsg, picojson::object& result)
{
	return false;
}

int LicenseManager::Activate(String name, String company, String licenseKey)
{
	return ACTIVATION_FAILED;
}

int LicenseManager::ActivateFromURL(String url)
{
    return ACTIVATION_FAILED;
}


bool LicenseManager::Deactivate()
{
    return false;
}

bool LicenseManager::RequestDemoTokens(String strMACAddr)
{
	return false;
}

#endif
    
} // namespace Zephyros
