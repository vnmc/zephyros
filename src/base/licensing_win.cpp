#include <iomanip>
#include <tchar.h>

#include <Shlwapi.h>
#include <ShlObj.h>
#include <Uxtheme.h>
//#include <WinInet.h>
#include <winhttp.h>
#include <vsstyle.h>

#include "app.h"
#include "client_handler.h"
#include "resource.h"
#include "licensing.h"
#include "dialog_win.h"
#include "file_util.h"
#include "string_util.h"


#define BUF_SIZE 512

extern HINSTANCE g_hInst;
extern LicenseManager* g_pLicenseManager;
extern CefRefPtr<ClientHandler> g_handler;


//////////////////////////////////////////////////////////////////////////
// LicenseData Implementation

LicenseData::LicenseData()
{
	// read the license data from the file

	TCHAR szFilename[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szFilename);

	PathAppend(szFilename, TEXT("\\Vanamco"));
	CreateDirectory(szFilename, NULL);

	PathAppend(szFilename, TEXT("\\"));
	PathAppend(szFilename, TEXT(APP_NAME));
	CreateDirectory(szFilename, NULL);

	PathAppend(szFilename, TEXT("\\data.t2"));

	HANDLE hFile = CreateFile(szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// if the file doesn't exist in the default location, try the /res folder within the program directory
	if (hFile == INVALID_HANDLE_VALUE) 
	{
		Path path;
		FileUtil::GetApplicationResourcesPath(path);
		_tcscpy(szFilename, path.GetPath().c_str());
		PathAppend(szFilename, TEXT("\\data.t2"));

		hFile = CreateFile(szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD numBytesRead = 0;
		DWORD len = 0;
		TCHAR* buf = NULL;

		// read demo tokens
		DWORD numDemoTokens = 0;
		ReadFile(hFile, &numDemoTokens, sizeof(DWORD), &numBytesRead, NULL);
		for (int i = 0; i < (int) numDemoTokens; ++i)
		{
			ReadFile(hFile, &len, sizeof(DWORD), &numBytesRead, NULL);
			if (len <= 0 || len > 500)
				break;

			buf = new TCHAR[len];
			ReadFile(hFile, buf, len * sizeof(TCHAR), &numBytesRead, NULL);
			m_demoTokens.push_back(String(buf, (String::size_type) len));
			delete[] buf;
		}

		// read time stamp
		ReadFile(hFile, &len, sizeof(DWORD), &numBytesRead, NULL);
		if (0 < len && len < 500)
		{
			buf = new TCHAR[len];
			ReadFile(hFile, buf, len * sizeof(TCHAR), &numBytesRead, NULL);
			m_timestampLastDemoTokenUsed = String(buf, (String::size_type) len);
			delete[] buf;
		}

		// read activation cookie
		ReadFile(hFile, &len, sizeof(DWORD), &numBytesRead, NULL);
		if (0 < len && len < 500)
		{
			buf = new TCHAR[len];
			ReadFile(hFile, buf, len * sizeof(TCHAR), &numBytesRead, NULL);
			m_activationCookie = String(buf, (String::size_type) len);
			delete[] buf;
		}

		// read license key
		if (ReadFile(hFile, &len, sizeof(DWORD), &numBytesRead, NULL))
		{	
			if (numBytesRead > 0 && 0 < len && len < 500)
			{
				buf = new TCHAR[len];
				ReadFile(hFile, buf, len * sizeof(TCHAR), &numBytesRead, NULL);
				m_licenseKey = LicenseData::Decrype(String(buf, (String::size_type) len));
				delete[] buf;
			}
		}

		// read name
		if (ReadFile(hFile, &len, sizeof(DWORD), &numBytesRead, NULL))
		{
			if (numBytesRead > 0 && 0 < len && len < 500)
			{
				buf = new TCHAR[len];
				ReadFile(hFile, buf, len * sizeof(TCHAR), &numBytesRead, NULL);
				m_name = LicenseData::Decrype(String(buf, (String::size_type) len));
				delete[] buf;
			}
		}

		// read company
		if (ReadFile(hFile, &len, sizeof(DWORD), &numBytesRead, NULL))
		{
			if (numBytesRead > 0 && 0 < len && len < 500)
			{
				buf = new TCHAR[len];
				ReadFile(hFile, buf, len * sizeof(TCHAR), &numBytesRead, NULL);
				m_company = LicenseData::Decrype(String(buf, (String::size_type) len));
				delete[] buf;
			}
		}

		CloseHandle(hFile);
	}
}

void LicenseData::Save()
{
	// save the license data to the file

	TCHAR szFilename[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szFilename);

	PathAppend(szFilename, TEXT("\\Vanamco"));
	CreateDirectory(szFilename, NULL);

	PathAppend(szFilename, TEXT("\\"));
	PathAppend(szFilename, TEXT(APP_NAME));
	CreateDirectory(szFilename, NULL);

	PathAppend(szFilename, TEXT("\\data.t2"));

	HANDLE hFile = CreateFile(szFilename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		hFile = CreateFile(szFilename, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_HIDDEN, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD numBytesWritten;
		DWORD len;

		// write demo tokens
		DWORD numDemoTokens = (DWORD) m_demoTokens.size();
		WriteFile(hFile, &numDemoTokens, sizeof(DWORD), &numBytesWritten, NULL);
		for (String token : m_demoTokens)
		{
			len = (DWORD) token.length();
			WriteFile(hFile, &len, sizeof(DWORD), &numBytesWritten, NULL);
			WriteFile(hFile, token.c_str(), (DWORD) (token.length() * sizeof(TCHAR)), &numBytesWritten, NULL);
		}

		// write time stamp
		len = (DWORD) m_timestampLastDemoTokenUsed.length();
		WriteFile(hFile, &len, sizeof(DWORD), &numBytesWritten, NULL);
		WriteFile(hFile, m_timestampLastDemoTokenUsed.c_str(), (DWORD) (m_timestampLastDemoTokenUsed.length() * sizeof(TCHAR)), &numBytesWritten, NULL);

		// write activation cookie
		len = (DWORD) m_activationCookie.length();
		WriteFile(hFile, &len, sizeof(DWORD), &numBytesWritten, NULL);
		WriteFile(hFile, m_activationCookie.c_str(), (DWORD) (m_activationCookie.length() * sizeof(TCHAR)), &numBytesWritten, NULL);

		// write license key
		String licenseKey = LicenseData::Encrype(m_licenseKey);
		len = (DWORD) licenseKey.length();
		WriteFile(hFile, &len, sizeof(DWORD), &numBytesWritten, NULL);
		WriteFile(hFile, licenseKey.c_str(), (DWORD) (licenseKey.length() * sizeof(TCHAR)), &numBytesWritten, NULL);

		// write name
		String name = LicenseData::Encrype(m_name);
		len = (DWORD) name.length();
		WriteFile(hFile, &len, sizeof(DWORD), &numBytesWritten, NULL);
		WriteFile(hFile, name.c_str(), (DWORD) (name.length() * sizeof(TCHAR)), &numBytesWritten, NULL);

		// write company
		String company = LicenseData::Encrype(m_company);
		len = (DWORD) company.length();
		WriteFile(hFile, &len, sizeof(DWORD), &numBytesWritten, NULL);
		WriteFile(hFile, company.c_str(), (DWORD) (company.length() * sizeof(TCHAR)), &numBytesWritten, NULL);

		CloseHandle(hFile);
	}
}

String LicenseData::Now()
{
	String result = TEXT("");

	HCRYPTPROV hCryptProv;
    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_DSS, CRYPT_VERIFYCONTEXT))
        return TEXT("");

    HCRYPTHASH hHash;
    if (CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hHash))
    {
		// get the current time
		SYSTEMTIME time;
		GetLocalTime(&time);
		
		std::stringstream ss;
		ss << time.wYear << "-" << time.wMonth << "-" << time.wDay << "-" << time.wDayOfWeek;
		std::string strTimestamp = ss.str();

		// add the hash data
		if (!CryptHashData(hHash, (BYTE*) strTimestamp.c_str(), (DWORD) strTimestamp.length(), 0))
			App::ShowErrorMessage();

		// extract the hash data and construct the result string
		DWORD dwHashLen = 0;
		if (!CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0))
			App::ShowErrorMessage();
		BYTE* pData = new BYTE[dwHashLen];
		if (!CryptGetHashParam(hHash, HP_HASHVAL, pData, &dwHashLen, 0))
			App::ShowErrorMessage();

		StringStream ssResult;
		ssResult << std::hex << std::setfill(TEXT('0'));
		for (int i = 0; i < (int) dwHashLen; ++i)
			ssResult << std::setw(2) << pData[i];
		result = ssResult.str();

		delete[] pData;
		CryptDestroyHash(hHash);
	}

	CryptReleaseContext(hCryptProv, 0);
	return result;
}


//////////////////////////////////////////////////////////////////////////
// LicenseManager Implementation

LicenseManager::LicenseManager()
	: m_timerId(-1), m_pDemoDlg(NULL)
{
}

LicenseManager::~LicenseManager()
{
	KillTimer();
}

bool LicenseManager::VerifyKey(String key, String info, bool usePrevVersionPubKey)
{
	HCRYPTPROV hCryptProv;
    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_DSS, CRYPT_VERIFYCONTEXT))
        return false;

    bool result = false;

    BYTE szDERPubKey[2048];
    DWORD dwDERPubKeyLen = 2048;
    CERT_PUBLIC_KEY_INFO* publicKeyInfo = NULL;
    DWORD dwPublicKeyInfoLen = 0;

    if (CryptStringToBinary(
#ifdef PUBKEY_PREVVERSION
    		usePrevVersionPubKey ? PUBKEY_PREVVERSION : PUBKEY,
    		(DWORD) _tcslen(usePrevVersionPubKey ? PUBKEY_PREVVERSION : PUBKEY),
#else
    		PUBKEY, (DWORD) _tcslen(PUBKEY),
#endif
    		CRYPT_STRING_BASE64, szDERPubKey, &dwDERPubKeyLen, NULL, NULL) &&
		CryptDecodeObjectEx(X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, szDERPubKey, dwDERPubKeyLen, CRYPT_ENCODE_ALLOC_FLAG, NULL, &publicKeyInfo, &dwPublicKeyInfoLen))
    {
        HCRYPTKEY hPubKey;
        if (CryptImportPublicKeyInfo(hCryptProv, X509_ASN_ENCODING, publicKeyInfo, &hPubKey))
        {
            HCRYPTHASH hHash;
            if (CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hHash))
            {
				const TCHAR* szInfo = info.c_str();
				int infoLen = (int) info.length();

				int mbLen = WideCharToMultiByte(CP_UTF8, 0, szInfo, infoLen, NULL, 0, NULL, NULL);
				if (mbLen > 0)
				{
					BYTE* buf = new BYTE[mbLen];
					mbLen = WideCharToMultiByte(CP_UTF8, 0, szInfo, infoLen, (LPSTR) buf, mbLen, NULL, NULL);
					if (mbLen > 0)
					{
						CryptHashData(hHash, buf, mbLen, 0);

						BYTE* pKey = NULL;
						size_t keyLen = 0;
						if (DecodeKey(key, &pKey, &keyLen))
						{
							BYTE* pDSASignature = NULL;
							DWORD dwDSASignatureLen = 0;
							if (CryptDecodeObjectEx(X509_ASN_ENCODING, X509_DSS_SIGNATURE, pKey, (DWORD) keyLen, CRYPT_ENCODE_ALLOC_FLAG, NULL, &pDSASignature, &dwDSASignatureLen))
							{
								if (CryptVerifySignature(hHash, pDSASignature, dwDSASignatureLen, hPubKey, NULL, 0))
									result = true;

								LocalFree(pDSASignature);
							}

							delete[] pKey;
						}
					}

					delete[] buf;
				}

				CryptDestroyHash(hHash);
			}
			
			CryptDestroyKey(hPubKey);
		}
		
		LocalFree(publicKeyInfo);
    }

    CryptReleaseContext(hCryptProv, 0);

    return result;
}


class UpgradeDialog : public Dialog
{
public:
	UpgradeDialog(LicenseManager* pMgr, HWND hwndParent = NULL)
	  : Dialog(Zephyros::GetString(ZS_PREVVERSIONDLG_WNDTITLE), hwndParent),
		m_pLicenseManager(pMgr)
	{
		ON_COMMAND(IDOK, UpgradeDialog::OnOK);
		ON_COMMAND(IDCANCEL, UpgradeDialog::OnCancel);
		ON_MESSAGE(WM_PAINT, UpgradeDialog::OnPaint);
		ON_MESSAGE(WM_INITDIALOG, UpgradeDialog::OnInitDialog);

    	AddStatic(Zephyros::GetString(ZS_PREVVERSIONDLG_TITLE), 105, 19, 185, 8);
    	AddStatic(Zephyros::GetString(ZS_PREVVERSIONDLG_DESCRIPTION), 105, 40, 185, 30);
    	AddDefaultButton(Zephyros::GetString(ZS_PREVVERSIONDLG_UPGRADE), 171, 155, 131, 14, IDOK);
    	AddButton(Zephyros::GetString(ZS_PREVVERSIONDLG_BACK), 115, 155, 50, 14, IDCANCEL);
	}

	~UpgradeDialog()
	{
		if (m_hIcon != NULL)
			DestroyIcon(m_hIcon);
	}

protected:
	void OnOK(WORD w, LPARAM l)
	{
		// open URL
		m_pLicenseManager->OpenUpgradeLicenseURL();
	}

	void OnCancel(WORD w, LPARAM l)
	{
		Dismiss(IDCANCEL);
	}

	void OnInitDialog(WPARAM wParam, LPARAM lParam)
	{
		// set the dialog position on the screen
		RECT r;
		GetWindowRect(GetDesktopWindow(), &r);
		LONG width = r.right - r.left;
		LONG height = r.bottom - r.top;
		GetWindowRect(m_hwnd, &r);
		SetWindowPos(m_hwnd, NULL, (width - r.right + r.left) / 2, (height - r.bottom + r.top) / 2 - 80, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	void OnPaint(WPARAM wParam, LPARAM lParam)
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(m_hwnd, &ps);

		// paint the application icon
		if (m_hIcon == NULL)
			m_hIcon = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_CEFCLIENT), IMAGE_ICON, 128, 128, LR_DEFAULTCOLOR);
		if (m_hIcon != NULL)
			DrawIconEx(hDC, 16, 10, m_hIcon, 128, 128, 0, NULL, DI_NORMAL);

		// clean up
		EndPaint(m_hwnd, &ps);
	}

private:
	void Dismiss(INT_PTR result)
	{
		m_pLicenseManager = NULL;
		EndDialog(m_hwnd, result);
	}

private:
	LicenseManager* m_pLicenseManager;
	HICON m_hIcon;
};

class LicenseKeyDialog : public Dialog
{
public:
	LicenseKeyDialog(LicenseManager* pMgr, HWND hwndParent = NULL)
	  : Dialog(Zephyros::GetString(ZS_ENTERLICKEYDLG_WNDTITLE), hwndParent),
		m_pLicenseManager(pMgr)
	{
		ON_COMMAND(IDOK, LicenseKeyDialog::OnOK);
		ON_COMMAND(IDCANCEL, LicenseKeyDialog::OnCancel);

    	AddGroupBox(Zephyros::GetString(ZS_ENTERLICKEYDLG_TITLE), 7, 7, 295, 117, -1, WS_CHILD | WS_VISIBLE | BS_CENTER);
    	AddStatic(Zephyros::GetString(ZS_ENTERLICKEYDLG_FULL_NAME), 37, 33, 34, 8, -1, WS_CHILD | WS_VISIBLE | SS_RIGHT);
    	AddTextBox(TEXT(""), 87, 30, 195, 14, IDC_EDIT_FULLNAME, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL);
    	AddStatic(Zephyros::GetString(ZS_ENTERLICKEYDLG_ORGANIZATION), 27, 55, 44, 8, -1, WS_CHILD | WS_VISIBLE | SS_RIGHT);
    	AddTextBox(TEXT(""), 87, 52, 195, 14, IDC_EDIT_ORGANIZATION, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL);
    	AddStatic(Zephyros::GetString(ZS_ENTERLICKEYDLG_LICKEY), 30, 77, 41, 8, -1, WS_CHILD | WS_VISIBLE | SS_RIGHT);
    	AddTextBox(TEXT(""), 87, 74, 195, 33, IDC_EDIT_LICENSEKEY, WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL);
    	AddDefaultButton(Zephyros::GetString(ZS_ENTERLICKEYDLG_ACTIVATE), 198, 140, 50, 14, IDOK);
    	AddButton(Zephyros::GetString(ZS_ENTERLICKEYDLG_CANCEL), 252, 140, 50, 14, IDCANCEL);
	}

protected:
	void OnOK(WORD w, LPARAM l)
	{
		TCHAR buf[BUF_SIZE + 1];

		GetDlgItemText(m_hwnd, IDC_EDIT_FULLNAME, buf, BUF_SIZE);
		String fullName = buf;
		GetDlgItemText(m_hwnd, IDC_EDIT_ORGANIZATION, buf, BUF_SIZE);
		String organization = buf;
		GetDlgItemText(m_hwnd, IDC_EDIT_LICENSEKEY, buf, BUF_SIZE);
		String key = buf;

		if (m_pLicenseManager->Activate(fullName, organization, key))
			Dismiss(IDOK);
		else if (m_pLicenseManager->IsObsoleteLicense(fullName, organization, key))
		{
			Dismiss(IDOK);
			UpgradeDialog dlg(m_pLicenseManager, m_hwnd);
			dlg.DoModal();
		}
	}

	void OnCancel(WORD w, LPARAM l)
	{
		Dismiss(IDCANCEL);
	}

private:
	void Dismiss(INT_PTR result)
	{
		m_pLicenseManager = NULL;
		EndDialog(m_hwnd, result);
	}

private:
	LicenseManager* m_pLicenseManager;
};


class DemoDialog : public Dialog
{
public:
	DemoDialog(LicenseManager* pMgr, int numDaysLeft)
	  : Dialog(Zephyros::GetString(ZS_DEMODLG_WNDTITLE)),
		m_pLicenseManager(pMgr), m_numDaysLeft(numDaysLeft),
		m_hFontBold(NULL), m_hIcon(NULL)
	{
		ON_MESSAGE(WM_INITDIALOG, DemoDialog::OnInitDialog);
		ON_MESSAGE(WM_PAINT, DemoDialog::OnPaint);

		ON_COMMAND(IDOK, DemoDialog::OnContinueDemo);
		ON_COMMAND(IDC_PURCHASELICENSE, DemoDialog::OnPurchaseLicense);
		ON_COMMAND(IDC_ENTERLICENSEKEY, DemoDialog::OnEnterLicenseKey);
		ON_COMMAND(IDCANCEL, DemoDialog::OnCancel);

    	AddStatic(Zephyros::GetString(ZS_DEMODLG_TITLE), 105, 19, 185, 8, IDC_DEMOTITLE);
    	AddStatic(Zephyros::GetString(ZS_DEMODLG_DESCRIPTION), 105, 40, 185, 30);
    	AddGroupBox(Zephyros::GetString(ZS_DEMODLG_REMAINING_TIME), 18, 92, 272, 59, -1, WS_CHILD | WS_VISIBLE | BS_CENTER);
    	AddStatic(Zephyros::GetString(ZS_DEMODLG_REMAINING_TIME_DESC), 33, 107, 210, 8);
    	AddStatic(TEXT(""), 33, 132, 243, 8, IDC_TEXT_DAYS);

    	AddDefaultButton(TEXT(""), 18, 172, 80, 14, IDOK);
    	AddButton(Zephyros::GetString(ZS_DEMODLG_PURCHASE_LICENSE), 114, 172, 80, 14, IDC_PURCHASELICENSE);
    	AddButton(Zephyros::GetString(ZS_DEMODLG_ENTER_LICKEY), 210, 172, 80, 14, IDC_ENTERLICENSEKEY);
	}

	~DemoDialog()
	{
		if (m_hFontBold != NULL)
			DeleteObject(m_hFontBold);
		if (m_hIcon != NULL)
			DestroyIcon(m_hIcon);
	}

	void Dismiss(INT_PTR result)
	{
		m_pLicenseManager = NULL;
		// TODO: use PostMessage() if in Worker thread
		EndDialog(m_hwnd, result);
	}

protected:
	void OnInitDialog(WPARAM wParam, LPARAM lParam)
	{
		// set the dialog position on the screen
		RECT r;
		GetWindowRect(GetDesktopWindow(), &r);
		LONG width = r.right - r.left;
		LONG height = r.bottom - r.top;
		GetWindowRect(m_hwnd, &r);
		SetWindowPos(m_hwnd, NULL, (width - r.right + r.left) / 2, (height - r.bottom + r.top) / 2 - 80, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		// set the text of the number of days display
		String captionNumDays = m_pLicenseManager->GetDaysCountLabelText();
		SetDlgItemText(m_hwnd, IDC_TEXT_DAYS, captionNumDays.c_str());

		// set the caption of the demo button
		String captionDemoButton = m_pLicenseManager->GetDemoButtonCaption();
		SetDlgItemText(m_hwnd, IDOK, captionDemoButton.c_str());

		// make the title bold
		HFONT hFont = (HFONT) SendDlgItemMessage(m_hwnd, IDC_DEMOTITLE, WM_GETFONT, 0, 0);
		LOGFONT font;
		GetObject(hFont, sizeof(LOGFONT), &font);
		font.lfWeight = FW_BOLD;
		m_hFontBold = CreateFontIndirect(&font);
		SendDlgItemMessage(m_hwnd, IDC_DEMOTITLE, WM_SETFONT, (WPARAM) m_hFontBold, TRUE);
	}

	void OnPaint(WPARAM wParam, LPARAM lParam)
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(m_hwnd, &ps);

		// paint the application icon
		if (m_hIcon == NULL)
			m_hIcon = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_CEFCLIENT), IMAGE_ICON, 128, 128, LR_DEFAULTCOLOR);
		if (m_hIcon != NULL)
			DrawIconEx(hDC, 16, 10, m_hIcon, 128, 128, 0, NULL, DI_NORMAL);

		// paint the meter bar
		HTHEME theme = OpenThemeData(m_hwnd, TEXT("PROGRESS"));

		// set the fill state of the meter
		WPARAM fillstate = PBFS_PARTIAL;
		if (m_numDaysLeft <= 3)
			fillstate = PBFS_PAUSED;
		if (m_numDaysLeft <= 1)
			fillstate = PBFS_ERROR;

		// find the rect where to paint the meter
		RECT r;
		GetWindowRect(GetDlgItem(m_hwnd, IDC_TEXT_DAYS), &r);
		ScreenToClient(m_hwnd, (POINT*) &(r.left));
		ScreenToClient(m_hwnd, (POINT*) &(r.right));
		LONG offs = r.bottom - r.top + 2;
		r.top -= offs;
		r.bottom -= offs;

		// draw the background
		DrawThemeBackground(theme, hDC, PP_TRANSPARENTBAR, PBBS_PARTIAL, &r, NULL);

		// draw the filling
		r.right = r.left + ((r.right - r.left) * (NUMBER_OF_DEMO_DAYS - m_numDaysLeft)) / NUMBER_OF_DEMO_DAYS;
		DrawThemeBackground(theme, hDC, PP_FILL, fillstate, &r, NULL);

		// clean up
		CloseThemeData(theme);
		EndPaint(m_hwnd, &ps);
	}

	void OnContinueDemo(WORD w, LPARAM l)
	{
		bool canContinue = m_numDaysLeft > 0;
		if (canContinue)
			canContinue = m_pLicenseManager->ContinueDemo();

		Dismiss(canContinue ? IDOK : IDCANCEL);
	}

	void OnPurchaseLicense(WORD w, LPARAM l)
	{
		m_pLicenseManager->OpenPurchaseLicenseURL();
	}

	void OnEnterLicenseKey(WORD w, LPARAM l)
	{
		LicenseKeyDialog dlg(m_pLicenseManager, m_hwnd);
		dlg.DoModal();
	}

	void OnCancel(WORD w, LPARAM l)
	{
		Dismiss(IDCANCEL);
	}

private:
	LicenseManager* m_pLicenseManager;

	int m_numDaysLeft;

	HICON m_hIcon;
	HFONT m_hFontBold;
};


void CALLBACK DemoTimeout(HWND hwnd, UINT msg, UINT_PTR event, DWORD time)
{
	if (g_pLicenseManager->CheckDemoValidity())
		g_pLicenseManager->KillTimer();
}

void LicenseManager::KillTimer()
{
	if (m_timerId == -1)
		return;

	::KillTimer(NULL, m_timerId);
	m_timerId = -1;
}

void LicenseManager::ShowDemoDialog()
{
	// cancel any previous timer
	KillTimer();

	// show the demo dialog
	m_pDemoDlg = new DemoDialog(this, GetNumDaysLeft());
	INT_PTR result = m_pDemoDlg->DoModal();
	delete m_pDemoDlg;
	m_pDemoDlg = NULL;
	m_canStartApp = result == IDOK;

	// check the validity of the demo after 6 hours
	if (m_canStartApp && !IsActivated())
		m_timerId = SetTimer(NULL, 0, 6 * 3600 * 1000, DemoTimeout);

	// if the message loop is already running, post a quit message
	if (!m_canStartApp)
		App::QuitMessageLoop();
}

void LicenseManager::ShowEnterLicenseDialog()
{
	LicenseKeyDialog dlg(this, g_handler->GetMainHwnd());
	dlg.DoModal();
}

void LicenseManager::OpenPurchaseLicenseURL()
{
	SHELLEXECUTEINFO execInfo = { 0 };

	execInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	execInfo.fMask = SEE_MASK_DEFAULT;
	execInfo.lpVerb = TEXT("open");
	execInfo.lpFile = SHOP_URL;
	execInfo.nShow = SW_SHOW;

	ShellExecuteEx(&execInfo);
}

void LicenseManager::OpenUpgradeLicenseURL()
{
	SHELLEXECUTEINFO execInfo = { 0 };

	execInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	execInfo.fMask = SEE_MASK_DEFAULT;
	execInfo.lpVerb = TEXT("open");
	execInfo.lpFile = UPGRADE_URL;
	execInfo.nShow = SW_SHOW;

	ShellExecuteEx(&execInfo);
}

bool LicenseManager::SendRequest(String strUrlPath, std::string strPostData, std::stringstream& out)
{
	bool ret = false;

	HINTERNET hSession = WinHttpOpen(TEXT(APP_NAME), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession != NULL)
	{
		HINTERNET hHttp = WinHttpConnect(hSession, LICENSING_HOST, LICENSING_PORT, 0);
		if (hHttp != NULL)
		{
			HINTERNET hRequest = WinHttpOpenRequest(hHttp, TEXT("POST"), strUrlPath.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
			if (hRequest != NULL)
			{
				// check for proxy
				bool setProxySucceeded = false;

				WINHTTP_PROXY_INFO proxyInfo = { 0 };
				WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig = { 0 };

				if (WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig))
				{
					if (ieProxyConfig.fAutoDetect || ieProxyConfig.lpszAutoConfigUrl != NULL)
					{
						// auto detect proxy options
						WINHTTP_AUTOPROXY_OPTIONS autoproxyOption = { 0 };
						autoproxyOption.dwFlags = 0;
						autoproxyOption.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
						autoproxyOption.fAutoLogonIfChallenged = TRUE;

						if (ieProxyConfig.fAutoDetect)
							autoproxyOption.dwFlags |= WINHTTP_AUTOPROXY_AUTO_DETECT;

						// fallback to a autoconfig URL if one has been set
						if (ieProxyConfig.lpszAutoConfigUrl != NULL)
						{
							autoproxyOption.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;
							autoproxyOption.lpszAutoConfigUrl = ieProxyConfig.lpszAutoConfigUrl;
						}

						// get the proxy for the URL we want to access and set the option
						StringStream ssUrl;
						ssUrl << LICENSING_PROTOCOL << TEXT("//") << LICENSING_HOST << TEXT(":") << LICENSING_PORT << strUrlPath;
						if (WinHttpGetProxyForUrl(hSession, ssUrl.str().c_str(), &autoproxyOption, &proxyInfo))
						{
							if (WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo)))
								setProxySucceeded = true;
						}
						else
						{
							// couldn't get autoproxy; assume we can use direct connection
							setProxySucceeded = true;
						}
					}
					else if (ieProxyConfig.lpszProxy != NULL)
					{
						// use fixed proxy options
						WINHTTP_PROXY_INFO p = { 0 };
						p.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
						p.lpszProxy = ieProxyConfig.lpszProxy;
						p.lpszProxyBypass = ieProxyConfig.lpszProxyBypass;

						if (WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY, &p, sizeof(p)))
							setProxySucceeded = true;
					}
					else
						setProxySucceeded = true;
				}
				else
				{
					// ignore the error and try to request directly
					setProxySucceeded = true;
				}

				// prepare and make the request
				if (setProxySucceeded)
				{
					StringStream ssHeaders;
					ssHeaders << TEXT("Content-Type: application/x-www-form-urlencoded\r\nContent-Length: ");
					ssHeaders << strPostData.length();

					String strHeaders = ssHeaders.str();
					char* szPostData = new char[strPostData.length() + 1];
					strcpy(szPostData, strPostData.c_str());

					if (WinHttpSendRequest(hRequest, strHeaders.c_str(), (DWORD) strHeaders.length(), szPostData, (DWORD) strPostData.length(), (DWORD) strPostData.length(), NULL))
					{
						if (WinHttpReceiveResponse(hRequest, NULL))
						{
							char szBuf[BUF_SIZE + 1]; 
							DWORD dwBytesRead = 0;

							while (WinHttpReadData(hRequest, szBuf, BUF_SIZE, &dwBytesRead))
							{
								if (dwBytesRead == 0)
									break;

								out << std::string(szBuf, szBuf + dwBytesRead);
								dwBytesRead = 0;
								ret = true;
							}
						}
					}

					delete[] szPostData;
				}

				if (ieProxyConfig.lpszProxy != NULL)
					GlobalFree(ieProxyConfig.lpszProxy);
				if (ieProxyConfig.lpszProxyBypass != NULL)
					GlobalFree(ieProxyConfig.lpszProxyBypass);
				if (ieProxyConfig.lpszAutoConfigUrl != NULL)
					GlobalFree(ieProxyConfig.lpszAutoConfigUrl);
				if (proxyInfo.lpszProxy != NULL)
					GlobalFree(proxyInfo.lpszProxy);
				if (proxyInfo.lpszProxyBypass != NULL)
					GlobalFree(proxyInfo.lpszProxyBypass);

				WinHttpCloseHandle(hRequest);
			}

			WinHttpCloseHandle(hHttp);
		}

		WinHttpCloseHandle(hSession);
	}
	
	return ret;
}

/*
bool LicenseManager::SendRequest(String strUrlPath, std::string strPostData, std::stringstream& out)
{
	bool ret = false;

	HINTERNET hSession = InternetOpen(TEXT(APP_NAME), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hSession != NULL)
	{
		HINTERNET hHttp = InternetConnect(hSession, LICENSING_HOST, LICENSING_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
		if (hHttp != NULL)
		{
			HINTERNET hRequest = HttpOpenRequest(hHttp, TEXT("POST"), strUrlPath.c_str(), NULL, NULL, NULL,
				INTERNET_FLAG_NO_AUTH | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD, NULL);

			if (hRequest != NULL)
			{
				StringStream ssHeaders;
				ssHeaders << TEXT("Content-Type: application/x-www-form-urlencoded\r\nContent-Length: ");
				ssHeaders << strPostData.length();

				String strHeaders = ssHeaders.str();
				char* szPostData = new char[strPostData.length() + 1];
				strcpy(szPostData, strPostData.c_str());

				if (HttpSendRequest(hRequest, strHeaders.c_str(), (DWORD) strHeaders.length(), szPostData, (DWORD) strPostData.length()))
				{
					char szBuf[BUF_SIZE + 1]; 
					DWORD dwBytesRead = 0;

					while (InternetReadFile(hRequest, szBuf, BUF_SIZE, &dwBytesRead))
					{
						if (dwBytesRead == 0)
							break;

						out << std::string(szBuf, szBuf + dwBytesRead);
						dwBytesRead = 0;
						ret = true;
					}
				}

				delete[] szPostData;

				InternetCloseHandle(hRequest);
			}

			InternetCloseHandle(hHttp);
		}

		InternetCloseHandle(hSession);
	}
	
	return ret;
}
*/

void LicenseManager::OnActivate(bool isSuccess)
{
	if (m_pDemoDlg != NULL)
		m_pDemoDlg->Dismiss(isSuccess ? IDOK : IDCANCEL);
}

void LicenseManager::OnReceiveDemoTokens(bool isSuccess)
{
	// nothing needs to be done here...
}

String LicenseManager::DecodeURI(String uri)
{
	DWORD dwSize = (DWORD) uri.length() + 1;
	TCHAR* decodedUri = new TCHAR[dwSize];

	// decode the URI component
	String uriNoPluses = StringReplace(uri, TEXT("+"), TEXT(" "));
	if (UrlUnescape((PTSTR) uriNoPluses.c_str(), decodedUri, &dwSize, 0) == E_POINTER)
	{
		delete[] decodedUri;
		decodedUri = new TCHAR[dwSize + 1];
		UrlUnescape((PTSTR) uriNoPluses.c_str(), decodedUri, &dwSize, 0);
	}

	// convert from UTF8 (we first have to convert to a non-wide string for MultiByteToWideChar to work)
	int nUriLen = (int) _tcslen(decodedUri);
	std::string strDecodedUri(decodedUri, decodedUri + nUriLen);
	int nWcLen = MultiByteToWideChar(CP_UTF8, 0, (LPCCH) strDecodedUri.c_str(), nUriLen, NULL, 0);
	TCHAR* buf = new TCHAR[nWcLen + 1];
	MultiByteToWideChar(CP_UTF8, 0, (LPCCH) strDecodedUri.c_str(), nUriLen, buf, nWcLen);
	buf[nWcLen] = 0;
	String result = String(buf, buf + nWcLen);
	
	delete[] decodedUri;
	delete[] buf;

	return result;
}