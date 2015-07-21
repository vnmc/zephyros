//
//  licensing.h
//

#ifndef Zephyros_licensing_h
#define Zephyros_licensing_h

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

#include "base/types.h"

#include "util/base32.h"
#include "util/picojson.h"

#include "native_extensions/network_util.h"
#include "native_extensions/os_util.h"


//////////////////////////////////////////////////////////////////////////
// Defines

#define ACTIVATION_SUCCEEDED 1
#define ACTIVATION_FAILED 0
#define ACTIVATION_OBSOLETELICENSE -1


//////////////////////////////////////////////////////////////////////////
// Type Definitions

namespace Zephyros {

class ReceiptChecker;
class LicenseManager;


typedef struct
{
    int productId;
    const TCHAR* pubkey;
} LicenseInfo;

typedef struct
{
    LicenseInfo currentLicenseInfo;
    
    LicenseInfo* prevLicenseInfo;
    int prevLicenseInfoCount;
    
    int numDemoDays;
    
    const TCHAR* demoTokensURL;
    const TCHAR* activationURL;
    const TCHAR* deactivationURL;
    
    const TCHAR* shopURL;
    const TCHAR* upgradeURL;
    
    const TCHAR* activationLinkPrefix;

    ReceiptChecker* pReceiptChecker;
    
    const TCHAR* dialogTitle;
    const TCHAR* dialogDescription;
    const TCHAR* dialogPrevLicenseHintTitle;
    const TCHAR* dialogPrevLicenseHintDescription;
} LicenseManagerInfo;
    
} // namespace Zephyros


//////////////////////////////////////////////////////////////////////////
// Classes

//#if APPSTORE == 0 || !defined(APPSTORE)
#ifdef OS_MACOSX
#if __OBJC__

@class LicenseCheckWindowController;

@interface LicenseManagerTimerDelegate : NSObject
{
    @public
    Zephyros::LicenseManager* m_pLicenseManager;
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


class LicenseData
{
	friend class LicenseManager;

public:
	LicenseData();
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
    
    static String Encrype(String s);
    static String Decrype(String s);

	std::vector<String> m_demoTokens;
	String m_timestampLastDemoTokenUsed;
	String m_activationCookie;
    String m_licenseKey;
    String m_name;
    String m_company;
    
    static char gheim[129];
};
    
    
class ReceiptChecker
{
public:
    virtual void CopyAppStoreReceipt();
    String GetReceiptFilename();
    virtual String GetAppStoreBundleName() = 0;

    virtual bool CheckReceipt() = 0;
};


class LicenseManager
{
public:
	LicenseManager(LicenseManagerInfo* info);
	~LicenseManager();
    
    inline int GetNumDemoDays() { return m_info->numDemoDays; }
    String GetDialogTitle();
    String GetDialogDescription();
    String GetDialogPrevLicenseHintTitle();
    String GetDialogPrevLicenseHintDescription();
    inline ReceiptChecker* GetReceiptChecker() { return m_info->pReceiptChecker; }

    /**
     * Starts the license manager.
     */
	inline void Start()
    {
        m_canStartApp = false;
        
#ifdef OS_WIN
        m_lastDemoValidityCheck = 0;
#endif
        
        if (!IsActivated())
            ShowDemoDialog();
        else
            m_canStartApp = true;
    }

	bool RequestDemoTokens(String strMACAddr = TEXT(""));
    
    String DecodeURI(String uri);

	int Activate(String name, String company, String licenseKey);
    int ActivateFromURL(String url);
    bool Deactivate();

    inline bool IsLicensingLink(String url)
    {
        return IsLicensingLink(url.c_str());
    }
    
    inline bool IsLicensingLink(const TCHAR* url)
    {
        if (!m_info->activationLinkPrefix || !url)
            return false;
        
        size_t len = _tcslen(m_info->activationLinkPrefix);
        return _tcsnccmp(url, m_info->activationLinkPrefix, len) == 0;
    }

    /**
     * Invoke this when the "Continue Demo" button was clicked on the demo dialog.
     * Returns true if the demo can be continued. If the method returns false, the
     * application should quit.
     */
    inline bool ContinueDemo()
    {
        // has the demo been started yet? if not, request demo tokens
        if (m_licenseData.m_demoTokens.size() == 0 && m_licenseData.m_timestampLastDemoTokenUsed == TEXT(""))
            return m_canStartApp = RequestDemoTokens();
        
        if (!m_licenseData.GetDemoToken() || !VerifyAll(m_licenseData.m_demoTokens.at(0), NetworkUtil::GetAllMACAddresses(), m_info->currentLicenseInfo.pubkey))
        {
            // invalid token; destroy demo tokens
            m_licenseData.m_demoTokens.clear();
            m_licenseData.Save();
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
	inline bool CheckDemoValidity()
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
        m_licenseData.GetDemoToken();
        ShowDemoDialog();
        return false;
    }
    
    void ShowEnterLicenseDialog();
    void OpenPurchaseLicenseURL();
    void OpenUpgradeLicenseURL();

	inline bool CanStartApp()
	{
		return m_canStartApp;
	}

    /**
     * Tests whether the product has been activated.
     * Returns true if the product was activated.
     */
	inline bool IsActivated()
    {
        bool ret =
            m_licenseData.m_activationCookie.length() > 0 &&
            VerifyAll(m_licenseData.m_activationCookie, NetworkUtil::GetAllMACAddresses(), m_info->currentLicenseInfo.pubkey);
        
#ifdef OS_MACOSX
        // on Mac, if the activation check wasn't successful, also check whether there
        // is a AppStore receipt, and validate it
        if (!ret)
            ret = CheckReceipt();
#endif
        
        // Try to activate with stored license
        if (!ret && m_licenseData.m_licenseKey != TEXT(""))
            ret = Activate(m_licenseData.m_name, m_licenseData.m_company, m_licenseData.m_licenseKey);
            
        return ret;
    }
    
    inline bool HasDemoTokens() { return m_licenseData.m_demoTokens.size() > 0; }
    
    inline const String GetFullName() { return String(m_licenseData.m_name); }
    inline const String GetCompany() { return String(m_licenseData.m_company); }
    inline const String GetLicenseKey() { return String(m_licenseData.m_licenseKey); }
    
    bool CheckReceipt();

	String GetDemoButtonCaption();
	String GetDaysCountLabelText();
    
private:
	inline int GetNumDaysLeft()
	{
		int numDaysLeft = (int) m_licenseData.m_demoTokens.size();
		if (m_licenseData.m_timestampLastDemoTokenUsed == TEXT("") || m_licenseData.m_timestampLastDemoTokenUsed != LicenseData::Now())
			numDaysLeft--;
		if (numDaysLeft < 0)
			numDaysLeft = m_licenseData.m_timestampLastDemoTokenUsed == TEXT("") ? m_info->numDemoDays : 0;

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

private:
    LicenseManagerInfo* m_info;
	LicenseData m_licenseData;
	bool m_canStartApp;

    
#if defined(OS_MACOSX)
    
public:
    LicenseCheckWindowControllerRef m_windowController;
private:
    LicenseManagerTimerDelegateRef m_timerDelegate;
    
#elif defined(OS_WIN)
    
public:
	void KillTimer();

	DemoDialog* m_pDemoDlg;

private:
	UINT_PTR m_timerId;
	ULONGLONG m_lastDemoValidityCheck;
    
#endif
};

//#endif  // APPSTORE == 0
   
} // namespace Zephyros


#endif // Zephyros_licensing_h
