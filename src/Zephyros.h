

#ifndef Zephyros_h
#define Zephyros_h

#include <string>
#include <vector>


//////////////////////////////////////////////////////////////////////////
// Constants

#define ACTIVATION_SUCCEEDED 1
#define ACTIVATION_FAILED 0
#define ACTIVATION_OBSOLETELICENSE -1


//////////////////////////////////////////////////////////////////////////
// Types

#ifdef OS_WIN

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <tchar.h>

#ifdef _UNICODE
typedef std::wstring String;
#else
typedef std::string String;
#endif

#define MAIN _tWinMain
#define MAIN_ARGS HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow
#define INIT_APPLICATION_ARGS hInstance, hPrevInstance, lpCmdLine, nCmdShow

#else

typedef std::string String;

#define TCHAR        char
#define TEXT(string) string

#define MAIN main
#define MAIN_ARGS int argc, const char* argv[]
#define INIT_APPLICATION_ARGS argc, argv

#endif  // OS_WIN


#ifdef USE_CEF

template<class T> class CefRefPtr;
class CefListValue;
class CefDictionaryValue;

#endif


#ifdef USE_WEBVIEW
#include "jsbridge_webview.h"
#endif


namespace Zephyros {

class NativeExtensions;


namespace JavaScript {

#ifdef USE_CEF

typedef CefRefPtr<CefDictionaryValue> Object;
typedef CefRefPtr<CefListValue> Array;

#endif

} // namespace JavaScript
} // namespace Zephyros

    
//////////////////////////////////////////////////////////////////////////
// Zephyros Structures

namespace Zephyros {

typedef struct
{
	TCHAR* szRegistryKey;
	int nIconID;
	int nMenuID;
	int nAccelID;
} WindowsInfo;

} // namespace Zephyros


//////////////////////////////////////////////////////////////////////////
// Zephyros Classes

namespace Zephyros {

// License Manager

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
    
    virtual JavaScript::Object GetLicenseInformation() = 0;

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

	virtual JavaScript::Object GetLicenseInformation();

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


//////////////////////////////////////////////////////////////////////////
// Zephyros Functions

namespace Zephyros {

int Run(MAIN_ARGS, const TCHAR* szAppName, const TCHAR* szAppVersion, const TCHAR* szAppURL);
int RunApplication(MAIN_ARGS);
void Shutdown();

const TCHAR* GetAppName();
const TCHAR* GetAppVersion();
const TCHAR* GetAppURL();

const TCHAR* GetCompanyName();
void SetCompanyName(const TCHAR* szCompanyName);

int GetResourceID(const TCHAR* szResourceName);
void SetResourceID(const TCHAR* szResourceName, int nID);

int GetMenuIDForCommand(const TCHAR* szCommand);
const TCHAR* GetMenuCommandForID(int nMenuID);
void SetMenuIDForCommand(const TCHAR* szCommand, int nMenuID);

WindowsInfo GetWindowsInfo();
void SetWindowsInfo(const TCHAR* szRegistryKey, int nIconID, int nMenuID, int nAccelID);

const TCHAR* GetCrashReportingURL();
const TCHAR* GetCrashReportingPrivacyPolicyURL();
void SetCrashReportingURL(const TCHAR* szReportingURL, const TCHAR* szPrivacyPolicyURL);
    
const TCHAR* GetUpdaterURL();
void SetUpdaterURL(const TCHAR* szURL);
    
AbstractLicenseManager* GetLicenseManager();
void SetLicenseManager(AbstractLicenseManager* pLicenseManager);

NativeExtensions* GetNativeExtensions();
void SetNativeExtensions(NativeExtensions* pNativeExtensions);
    
String GetString(int stringId);
void SetString(int stringId, String str);

} // namespace Zephyros


#endif
