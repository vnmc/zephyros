#ifdef OS_WIN
#include <tchar.h>
#endif

#include "base/zephyros_impl.h"
#include "base/app.h"
#include "base/licensing.h"

#include "native_extensions/network_util.h"
#include "native_extensions/os_util.h"


namespace Zephyros {

#ifndef APPSTORE

//////////////////////////////////////////////////////////////////////////
// LicenseData Implementation

char LicenseData::gheim[] = "zM#>Vj|wx+RG>)t/a@'Y>Pg^VmPK$Tq<<RWLz8.H2~8\\#m!s+;]Pk%vQoiVH7ILTAl:h~Bi9~8%c*aHK/O2JS2X&f%Z%B~&@8-W-C.V[HrrgvdE~70l?jxv*1^')t@1\"";

String LicenseData::Encrype(String s)
{
    StringStream ss;
    
    int j = 0;
    for (String::iterator it = s.begin(); it != s.end(); ++it, ++j)
        ss << (((int) (*it)) ^ LicenseData::gheim[j % 128]) << TEXT('.');
    
    return ss.str();
}

String LicenseData::Decrype(String s)
{
    StringStream ss;
    StringStream ssChar;
    
    int j = 0;
    for (String::iterator it = s.begin(); it != s.end(); ++it)
    {
        if (*it == TEXT('.'))
        {
            ss << (TCHAR) (_ttoi(ssChar.str().c_str()) ^ LicenseData::gheim[j % 128]);
            ssChar.str(String());
            ++j;
        }
        else
            ssChar << *it;
        
    }
    
    return ss.str();
}


//////////////////////////////////////////////////////////////////////////
// LicenseManager Implementation
    
String LicenseManager::GetDialogTitle()
{
    if (m_info->dialogTitle)
        return m_info->dialogTitle;
    
    StringStream ssTitle;
    ssTitle << TTEXT("Thank you for trying") << TEXT(" ") << Zephyros::GetAppName() << TEXT(".");
    return ssTitle.str();
}
    
String LicenseManager::GetDialogDescription()
{
    if (m_info->dialogDescription)
        return m_info->dialogDescription;
    
    StringStream ssDescription;
    ssDescription << TTEXT("You are using") << TEXT(" ") << Zephyros::GetAppName() << TEXT(" ") <<
        TTEXT("in demo mode.\nThe software will stop functioning when the end of the trial period is reached.");
    return ssDescription.str();
}
    
String LicenseManager::GetDialogPrevLicenseHintTitle()
{
    if (m_info->dialogPrevLicenseHintTitle)
        return m_info->dialogPrevLicenseHintTitle;
    return TTEXT("You need to upgrade your license.");
}
    
String LicenseManager::GetDialogPrevLicenseHintDescription()
{
    if (m_info->dialogPrevLicenseHintDescription)
        return m_info->dialogPrevLicenseHintDescription;
    return TTEXT("You have entered an obsolete license key.\nThis version requires a new license key.");
}

String LicenseManager::GetDemoButtonCaption()
{
	bool isDemoStarted = m_licenseData.m_timestampLastDemoTokenUsed != TEXT("");
	if (!isDemoStarted)
		return TTEXT("Start Demo");
	return GetNumDaysLeft() > 0 ? TTEXT("Continue Demo") : TTEXT("Close");
}

String LicenseManager::GetDaysCountLabelText()
{
	int numDaysLeft = GetNumDaysLeft();
	
	if (numDaysLeft > 1)
	{
		StringStream ss;
		ss << numDaysLeft << TTEXT(" days left.");
		return ss.str();
	}

	if (numDaysLeft == 1)
		return TTEXT("1 day left.");

	return TTEXT("No days left. Please purchase a license.");
}

bool LicenseManager::ParseResponse(String url, std::string data, String errorMsg, picojson::object& result)
{
	String title = Zephyros::g_szAppName;
	title += TEXT(" ");
	title += TTEXT("Licensing Error");

    String msg = errorMsg;

	std::stringstream out;
	if (!SendRequest(url, data, out))
    {
        msg.append(TTEXT("Failed to contact the licensing server.\n\nPlease try again later."));
        App::Alert(title, msg, App::AlertStyle::AlertError);
        return false;
    }
    
    picojson::value value;
	std::string err = picojson::parse(value, out);
	if (err.length() > 0)
	{
		msg.append(TTEXT("Failed to parse response from the licensing server.\n\nPlease try again later."));
		msg.append(err.begin(), err.end());
		App::Alert(title, msg, App::AlertStyle::AlertError);
		return false;
	}

	if (value.is<picojson::object>())
	{
		picojson::object objResponse = value.get<picojson::object>();
    
		if (objResponse.find("status") != objResponse.end() && objResponse["status"].get<std::string>() == "success")
		{
			result = objResponse;
			return true;
		}
    
		std::string respMsg = objResponse.find("message") != objResponse.end() ? objResponse["message"].get<std::string>() : "";
		msg.append(respMsg.begin(), respMsg.end());
		msg.append(TTEXT("\n\nPlease try again later."));
    
	    App::Alert(title, msg, App::AlertStyle::AlertError);
		return false;
	}

	msg.append(TTEXT("Unexpected response from the licensing server:\n"));
	std::string resp = out.str();
	msg.append(resp.begin(), resp.end());
	App::Alert(title, msg, App::AlertStyle::AlertError);
	return false;
}

/**
 * Activate the application.
 * If the activation cannot proceed, ACTIVATION_FAILED is returned. If the request to the
 * server is made, the method returns ACTIVATION_SUCCEEDED.
 * If an obsolete license was entered, the method returns ACTIVATION_OBSOLETELICENSE.
 */
int LicenseManager::Activate(String name, String company, String licenseKey)
{
	String title = Zephyros::g_szAppName;
	title += TEXT(" ");
	title += TTEXT("Activation");

	// verify that the entered data is OK
	StringStream ssInfo;
	ssInfo << name << TEXT("|") << company << TEXT("|") << m_info->currentLicenseInfo.productId;
	if (!VerifyKey(licenseKey, ssInfo.str(), m_info->currentLicenseInfo.pubkey))
	{
        // check if the user entered an obsolete license;
        // in case it's an obsolete license, don't alert user - notification about upgrade
        // is handled via separate dialog

        bool isObsoleteLicense = false;
        for (int i = 0; i < m_info->prevLicenseInfoCount; ++i)
        {
            StringStream ssInfoPrevVersion;
            ssInfoPrevVersion << name << TEXT("|") << company << TEXT("|") << m_info->prevLicenseInfo[i].productId;
            if (!VerifyKey(licenseKey, ssInfoPrevVersion.str(), m_info->prevLicenseInfo[i].pubkey))
            {
                isObsoleteLicense = true;
                break;
            }
        }
        
        if (!isObsoleteLicense)
        {
            App::Alert(title, TTEXT("This license key is not valid. Please try again."), App::AlertStyle::AlertError);
            return ACTIVATION_FAILED;
        }
        
		return ACTIVATION_OBSOLETELICENSE;
	}
    
	// add the POST data
	std::stringstream ssData;
	String strMACAddr = NetworkUtil::GetPrimaryMACAddress();
    String strOS = OSUtil::GetOSVersion();
	ssData <<
		"license=" << std::string(licenseKey.begin(), licenseKey.end()) <<
		"&eth0=" << std::string(strMACAddr.begin(), strMACAddr.end()) <<
		"&productcode=" << m_info->currentLicenseInfo.productId <<
        "&os=" << std::string(strOS.begin(), strOS.end());
	std::string strData = ssData.str();

	App::BeginWait();
    picojson::object result;

	String msg(TTEXT("Unfortunately, an error occurred during the activation of"));
	msg += Zephyros::g_szAppName;
	msg += TEXT(":\n\n");
    
	bool ret = ParseResponse(m_info->activationURL, strData, msg, result);
    App::EndWait();

    if (ret)
    {
        std::string cookie = result["activation"].get<std::string>();
		m_licenseData.m_activationCookie = String(cookie.begin(), cookie.end());
        m_licenseData.m_licenseKey = licenseKey;
        m_licenseData.m_name = name;
        m_licenseData.m_company = company;
		m_licenseData.Save();
        
		String msgSuccess(TTEXT("Thank you for activating "));
		msgSuccess += Zephyros::g_szAppName;
		msgSuccess += TEXT(".");
        App::Alert(title, msgSuccess, App::AlertStyle::AlertInfo);

#ifdef OS_WIN
        App::RemoveDemoMenuItems(App::GetMainHwnd());
#endif
    }

    m_canStartApp = ret;
    OnActivate(ret);

	return ACTIVATION_SUCCEEDED;
}

int LicenseManager::ActivateFromURL(String url)
{
	if (!LicenseManager::IsLicensingLink(url))
		return ACTIVATION_FAILED;
        
    StringStream ssUrl(url.c_str() + _tcslen(m_info->activationLinkPrefix));
    std::vector<String> parts;
    while (!ssUrl.eof())
    {
        // split the string at slashes
        String strPart;
        std::getline(ssUrl, strPart, TEXT('/'));
        parts.push_back(strPart);
    }
    
    if (parts.size() != 3)
        return ACTIVATION_FAILED;
    
    return Activate(DecodeURI(parts[0]), DecodeURI(parts[1]), DecodeURI(parts[2]));
}


bool LicenseManager::Deactivate()
{
    if (m_licenseData.m_activationCookie.length() == 0 || m_licenseData.m_licenseKey.length() == 0)
        return false;
    
    std::stringstream ssData;
    String strMACAddr = NetworkUtil::GetPrimaryMACAddress();
    ssData <<
        "license=" << std::string(m_licenseData.m_licenseKey.begin(), m_licenseData.m_licenseKey.end()) <<
        "&eth0=" << std::string(strMACAddr.begin(), strMACAddr.end()) <<
        "&productcode=" << m_info->currentLicenseInfo.productId;
    std::string strData = ssData.str();

    String msg(TTEXT("Unfortunately, an error occurred during deactivating"));
    msg += Zephyros::g_szAppName;
    msg += TEXT(":\n\n");

    picojson::object result;
    
    bool ret = ParseResponse(m_info->deactivationURL, strData, msg, result);
    
    if (ret)
    {
        // write new (empty) license data
        m_licenseData.m_activationCookie = TEXT("");
        m_licenseData.m_licenseKey = TEXT("");
        m_licenseData.m_name = TEXT("");
        m_licenseData.m_company = TEXT("");
        m_licenseData.Save();

        // show information message
        String title = Zephyros::g_szAppName;
        title += TEXT(" ");
        title += TTEXT("Deactivation");

        String msgSuccess(TTEXT("You have successfully deactivated "));
        msgSuccess += Zephyros::g_szAppName;
        msgSuccess += TEXT(".\n");
        msgSuccess += TTEXT("The application will quit now.");

        App::Alert(title, msgSuccess, App::AlertStyle::AlertInfo);
        App::QuitMessageLoop();
    }
    
    return ret;
}

/**
 * Request tokens to start the demo.
 * If requesting demo tokens cannot proceed, false is returned.
 * The method returns true if the request to the server is made.
 */
bool LicenseManager::RequestDemoTokens(String strMACAddr)
{
	// add the POST data
	std::stringstream ssData;
	if  (strMACAddr.length() == 0)
		strMACAddr = NetworkUtil::GetPrimaryMACAddress();
	String strOS = OSUtil::GetOSVersion();
	ssData <<
		"eth0=" << std::string(strMACAddr.begin(), strMACAddr.end()) <<
		"&productcode=" << m_info->currentLicenseInfo.productId <<
		"&os=" << std::string(strOS.begin(), strOS.end());
	std::string strData = ssData.str();

	App::BeginWait();
    picojson::object result;
	bool ret = ParseResponse(m_info->demoTokensURL, strData, TTEXT("Unfortunately, an error while retrieving a demo license from the licensing server:\n"), result);
    App::EndWait();
    
    if (ret)
    {
        // the response is a JSON object of the format { "status": <string>, "tokens": <array<string>> }
        if (result.find("tokens") != result.end())
		{
			picojson::array tokens = result["tokens"].get<picojson::array>();
			for (picojson::value token : tokens)
			{
				std::string strToken = token.get<std::string>();
				m_licenseData.m_demoTokens.push_back(String(strToken.begin(), strToken.end()));
			}
		}
        
		m_licenseData.Save();
        
		bool canContinueDemo = ContinueDemo();
		if (!canContinueDemo)
			m_canStartApp = false;
		OnReceiveDemoTokens(canContinueDemo);
    }
    else
        m_canStartApp = false;
    
	return ret;
}

#endif
    
} // namespace Zephyros
