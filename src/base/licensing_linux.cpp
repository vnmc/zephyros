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


#include <iomanip>
#include <iostream>
#include <fstream>

#include <signal.h>
#include <sys/time.h>

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/sha.h>

#include <gtk/gtk.h>

#include "zephyros_strings.h"

#include "base/app.h"
#include "base/licensing.h"
#include "base/cef/client_handler.h"

#include "util/string_util.h"

#include "native_extensions/browser.h"
#include "native_extensions/file_util.h"
#include "native_extensions/os_util.h"


extern CefRefPtr<Zephyros::ClientHandler> g_handler;

bool g_bIsDemoDialogShown = false;


namespace Zephyros {


//////////////////////////////////////////////////////////////////////////
// LicenseData Implementation

LicenseData::LicenseData(const TCHAR* szLicenseInformationFilename)
	: m_szLicenseInfoFilename(szLicenseInformationFilename)
{
	// read the license data from the file

	String strFilename(OSUtil::GetConfigDirectory());
	strFilename.append(TEXT("/"));
	strFilename.append(m_szLicenseInfoFilename);

    std::ifstream file;
    file.open(strFilename, std::ios::in | std::ios::binary);

    // if the file doesn't exist in the default location, try /etc/<license-file>
    if (!file.is_open())
    {
        strFilename = TEXT("/etc/");
        strFilename.append(m_szLicenseInfoFilename);

        file.open(strFilename, std::ios::in | std::ios::binary);
    }

    if (file.is_open())
    {
        int32_t nLen = 0;
        TCHAR* pBuf = NULL;

        // read demo tokens
        int32_t nNumDemoTokens = 0;
        file.read((char*) &nNumDemoTokens, sizeof(int32_t));
        for (int i = 0; i < (int) nNumDemoTokens; ++i)
        {
            file.read((char*) &nLen, sizeof(int32_t));
            if (nLen <= 0 || nLen > 500)
                break;

            pBuf = new TCHAR[nLen];
            file.read(pBuf, nLen);
            m_demoTokens.push_back(String(pBuf, (String::size_type) nLen));
            delete[] pBuf;
        }

        // read time stamp
        file.read((char*) &nLen, sizeof(int32_t));
        if (0 < nLen && nLen < 500)
        {
            pBuf = new TCHAR[nLen];
            file.read(pBuf, nLen * sizeof(TCHAR));
            m_timestampLastDemoTokenUsed = String(pBuf, (String::size_type) nLen);
            delete[] pBuf;
        }

        // read activation cookie
        file.read((char*) &nLen, sizeof(int32_t));
        if (0 < nLen && nLen < 500)
        {
            pBuf = new TCHAR[nLen];
            file.read(pBuf, nLen * sizeof(TCHAR));
            m_activationCookie = String(pBuf, (String::size_type) nLen);
            delete[] pBuf;
        }

        // read license key
        file.read((char*) &nLen, sizeof(int32_t));
        if (0 < nLen && nLen < 500)
        {
            pBuf = new TCHAR[nLen];
            file.read(pBuf, nLen * sizeof(TCHAR));
            m_licenseKey = LicenseData::Decrypt(String(pBuf, (String::size_type) nLen));
            delete[] pBuf;
        }

        // read name
        file.read((char*) &nLen, sizeof(int32_t));
        if (0 < nLen && nLen < 500)
        {
            pBuf = new TCHAR[nLen];
            file.read(pBuf, nLen * sizeof(TCHAR));
            m_name = LicenseData::Decrypt(String(pBuf, (String::size_type) nLen));
            delete[] pBuf;
        }

        // read company
        file.read((char*) &nLen, sizeof(int32_t));
        if (0 < nLen && nLen < 500)
        {
            pBuf = new TCHAR[nLen];
            file.read(pBuf, nLen * sizeof(TCHAR));
            m_company = LicenseData::Decrypt(String(pBuf, (String::size_type) nLen));
            delete[] pBuf;
        }

        file.close();
    }
}

void LicenseData::Save()
{
	// save the license data to the file

    String strFilename(OSUtil::GetConfigDirectory());
	strFilename.append(TEXT("/"));
	strFilename.append(m_szLicenseInfoFilename);

	 std::ofstream file;
    file.open(strFilename, std::ios::out | std::ios::binary);

    if (!file.is_open())
        return;

    // write demo tokens
    int32_t nLen = (int32_t) m_demoTokens.size();
    file.write((char*) &nLen, sizeof(int32_t));
    for (String strToken : m_demoTokens)
    {
        nLen = strToken.length();
        file.write((char*) &nLen, sizeof(int32_t));
        file.write(strToken.c_str(), nLen * sizeof(TCHAR));
    }

    // write time stamp
    nLen = (int32_t) m_timestampLastDemoTokenUsed.size();
    file.write((char*) &nLen, sizeof(int32_t));
    file.write(m_timestampLastDemoTokenUsed.c_str(), nLen * sizeof(TCHAR));

    // write activation cookie
    nLen = (int32_t) m_activationCookie.size();
    file.write((char*) &nLen, sizeof(int32_t));
    file.write(m_activationCookie.c_str(), nLen * sizeof(TCHAR));

    // write license key
    String strLicenseKey = LicenseData::Encrypt(m_licenseKey);
    nLen = (int32_t) strLicenseKey.size();
    file.write((char*) &nLen, sizeof(int32_t));
    file.write(strLicenseKey.c_str(), nLen * sizeof(TCHAR));

    // write name
    String strName = LicenseData::Encrypt(m_name);
    nLen = (int32_t) strName.size();
    file.write((char*) &nLen, sizeof(int32_t));
    file.write(strName.c_str(), nLen * sizeof(TCHAR));

    // write company
    String strCompany = LicenseData::Encrypt(m_company);
    nLen = (int32_t) strCompany.size();
    file.write((char*) &nLen, sizeof(int32_t));
    file.write(strCompany.c_str(), nLen * sizeof(TCHAR));

    file.close();
}

String LicenseData::Now()
{
    // get the current date string
    time_t t = time(0);
    struct tm *now = localtime(&t);
    std::stringstream ss;
    ss << (now->tm_year + 1900) << TEXT("-") << now->tm_mon << TEXT("-") << now->tm_mday << TEXT("-") << now->tm_wday;
    std::string strTimestamp = ss.str();

    // compute the hash
    unsigned char md[20];
    SHA1((unsigned char*) strTimestamp.c_str(), strTimestamp.length(), md);

    // format the hash
    StringStream ssResult;
    ssResult << std::hex << std::setfill(TEXT('0'));
	for (int i = 0; i < 20; ++i)
        ssResult << std::setw(2) << (int) md[i];

	return ssResult.str();
}


//////////////////////////////////////////////////////////////////////////
// LicenseManager Implementation

void SetTimeout(int nSeconds, void (*pFnx)(int nSigNum))
{
    struct sigaction sa = {0};
    sa.sa_handler = pFnx;
    sigaction(SIGVTALRM, &sa, NULL);

    itimerval timer = {0};
    timer.it_value.tv_sec = nSeconds;
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

void DemoTimeout(int nSigNum)
{
   	if (!Zephyros::GetLicenseManager()->CheckDemoValidity())
        SetTimeout(6 * 3600, DemoTimeout);
}

LicenseManagerImpl::LicenseManagerImpl()
{
	InitConfig();

	// initialize OpenSSL
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
}

LicenseManagerImpl::~LicenseManagerImpl()
{
    // shut down OpenSSL
    EVP_cleanup();
	ERR_free_strings();
}

bool LicenseManagerImpl::VerifyKey(String key, String info, const TCHAR* szPubkey)
{
    bool bResult = false;
    unsigned char* buf = NULL;
    size_t len = 0;

    String strPubKey = "-----BEGIN PUBLIC KEY-----\n";
    strPubKey.append(szPubkey);
    strPubKey.append("\n-----END PUBLIC KEY-----\n");

    if (DecodeKey(key, &buf, &len))
    {
        // read the PEM-encoded public key
        DSA* dsa = DSA_new();
        BIO *bio = BIO_new_mem_buf((void*) strPubKey.c_str(), -1);
        DSA* x = PEM_read_bio_DSA_PUBKEY(bio, &dsa, NULL, NULL);
        BIO_vfree(bio);

        if (dsa->pub_key)
        {
            // create the SHA1 hash of the licensing information
            unsigned char digest[20];
            SHA1((unsigned char*) info.c_str(), info.length(), digest);

            // verify the DSA signature
            bResult = DSA_verify(0, digest, 20, buf, len, dsa) > 0;
        }

        DSA_free(dsa);
        delete[] buf;
    }

    return bResult;
}

void LicenseManagerImpl::ShowDemoDialog()
{
    if (g_bIsDemoDialogShown)
        return;

	// create the demo dialog
    GtkWidget* pDialog = gtk_dialog_new_with_buttons(
        Zephyros::GetString(ZS_DEMODLG_WNDTITLE).c_str(),
        NULL,
        GTK_DIALOG_MODAL,
        GetDemoButtonCaption().c_str(), 1,
        NULL
    );

    // create dialog widgets
    GtkWidget* pIcon = gtk_image_new_from_file("");
    GtkWidget* pLabelTitle = gtk_label_new(Zephyros::GetString(ZS_DEMODLG_TITLE).c_str());
    GtkWidget* pLabelDescription = gtk_label_new(Zephyros::GetString(ZS_DEMODLG_DESCRIPTION).c_str());

    GtkWidget* pGroupRemainingTime = gtk_frame_new(Zephyros::GetString(ZS_DEMODLG_REMAINING_TIME).c_str());
    GtkWidget* pLabelRemainingTimeDescription = gtk_label_new(Zephyros::GetString(ZS_DEMODLG_REMAINING_TIME_DESC).c_str());
    GtkWidget* pLevelRemainingTime = gtk_level_bar_new();
    GtkWidget* pLabelRemainingTimeCount = gtk_label_new(GetDaysCountLabelText().c_str());

    // set label alignments
    gtk_misc_set_alignment((GtkMisc*) pLabelTitle, 0, 0.5);
    gtk_misc_set_alignment((GtkMisc*) pLabelDescription, 0, 0.5);
    gtk_misc_set_alignment((GtkMisc*) pLabelRemainingTimeDescription, 0, 0.5);
    gtk_misc_set_alignment((GtkMisc*) pLabelRemainingTimeCount, 0, 0.5);

    // layout
    GtkWidget* pBoxTitleTexts = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_box_pack_start(GTK_BOX(pBoxTitleTexts), pLabelTitle, FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(pBoxTitleTexts), pLabelDescription, FALSE, FALSE, 8);
    GtkWidget* pBoxTitle = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(pBoxTitle), pIcon, FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(pBoxTitle), pBoxTitleTexts, TRUE, TRUE, 8);
    GtkWidget* pBoxRemainingTime = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(pGroupRemainingTime), pBoxRemainingTime);
    gtk_box_pack_start(GTK_BOX(pBoxRemainingTime), pLabelRemainingTimeDescription, FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(pBoxRemainingTime), pLevelRemainingTime, FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(pBoxRemainingTime), pLabelRemainingTimeCount, FALSE, FALSE, 8);
    GtkWidget* pBoxMain = gtk_dialog_get_content_area(GTK_DIALOG(pDialog));
    gtk_box_pack_start(GTK_BOX(pBoxMain), pBoxTitle, FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(pBoxMain), pGroupRemainingTime, FALSE, FALSE, 8);

    // add buttons
    GtkWidget* pAreaAction = gtk_dialog_get_action_area(GTK_DIALOG(pDialog));

    // "purchase" button
    const TCHAR* szShopURL = static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetShopURL();
    if (szShopURL != NULL && szShopURL[0] != TCHAR('\0'))
    {
        GtkWidget* pButtonShop = gtk_button_new_with_label(Zephyros::GetString(ZS_DEMODLG_PURCHASE_LICENSE).c_str());
        g_signal_connect(pButtonShop, "clicked", [](){ Zephyros::GetLicenseManager()->OpenPurchaseLicenseURL(); }, NULL);
        gtk_box_pack_start(GTK_BOX(pAreaAction), pButtonShop, FALSE, FALSE, 8);
    }

    // enter license button
    GtkWidget* pButtonEnterLicense = gtk_button_new_with_label(Zephyros::GetString(ZS_DEMODLG_ENTER_LICKEY).c_str());
    g_signal_connect(pButtonEnterLicense, "clicked", [](){ Zephyros::GetLicenseManager()->ShowEnterLicenseDialog(); }, NULL);
    gtk_box_pack_start(GTK_BOX(pAreaAction), pButtonEnterLicense, FALSE, FALSE, 8);

    // destroy the dialog box after the user has responded
    g_signal_connect_swapped(pDialog, "response", G_CALLBACK(gtk_widget_destroy), pDialog);

    gtk_widget_show_all(pDialog);

    // show the demo dialog
    g_bIsDemoDialogShown = true;
    m_canStartApp = gtk_dialog_run(GTK_DIALOG(pDialog)) == 1 && ContinueDemo();
    g_bIsDemoDialogShown = false;

	// check the validity of the demo after 6 hours
	if (m_canStartApp && !IsActivated())
        SetTimeout(6 * 3600, DemoTimeout);

	// if the message loop is already running, post a quit message
	if (!m_canStartApp)
		App::QuitMessageLoop();
}

void LicenseManagerImpl::ShowEnterLicenseDialog()
{
    // create the demo dialog
    GtkWidget* pDialog = gtk_dialog_new_with_buttons(
        Zephyros::GetString(ZS_ENTERLICKEYDLG_WNDTITLE).c_str(),
        NULL,
        GTK_DIALOG_MODAL,
        Zephyros::GetString(ZS_ENTERLICKEYDLG_ACTIVATE).c_str(), GTK_RESPONSE_ACCEPT,
        Zephyros::GetString(ZS_ENTERLICKEYDLG_CANCEL).c_str(), GTK_RESPONSE_REJECT,
        NULL
    );

    // create dialog widgets
    GtkWidget* pGroup = gtk_frame_new(Zephyros::GetString(ZS_ENTERLICKEYDLG_TITLE).c_str());
    GtkWidget* pLabelName = gtk_label_new(Zephyros::GetString(ZS_ENTERLICKEYDLG_FULL_NAME).c_str());
    GtkWidget* pTextName = gtk_entry_new();
    GtkWidget* pLabelOrganization = gtk_label_new(Zephyros::GetString(ZS_ENTERLICKEYDLG_ORGANIZATION).c_str());
    GtkWidget* pTextOrganization = gtk_entry_new();
    GtkWidget* pLabelLicenseKey = gtk_label_new(Zephyros::GetString(ZS_ENTERLICKEYDLG_LICKEY).c_str());
    GtkWidget* pTextLicenseKey = gtk_entry_new();

    // set label alignments
    gtk_misc_set_alignment((GtkMisc*) pLabelName, 1, 0.5);
    gtk_misc_set_alignment((GtkMisc*) pLabelOrganization, 1, 0.5);
    gtk_misc_set_alignment((GtkMisc*) pLabelLicenseKey, 1, 0.5);

    // add buttons


    // layout
    GtkWidget* pGrid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(pGrid), pLabelName, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(pGrid), pTextName, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(pGrid), pLabelOrganization, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(pGrid), pTextOrganization, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(pGrid), pLabelLicenseKey, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(pGrid), pTextLicenseKey, 1, 2, 1, 1);
    gtk_container_add(GTK_CONTAINER(pGroup), pGrid);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(pDialog))), pGroup, FALSE, FALSE, 8);

    gtk_widget_show_all(pDialog);
    int ret = gtk_dialog_run(GTK_DIALOG(pDialog));
    if (ret == GTK_RESPONSE_ACCEPT)
    {
		int ret = Activate(
            gtk_entry_get_text(GTK_ENTRY(pTextName)),
            gtk_entry_get_text(GTK_ENTRY(pTextOrganization)),
            gtk_entry_get_text(GTK_ENTRY(pTextLicenseKey))
        );

/*
		switch (ret)
		{
		case ACTIVATION_SUCCEEDED:
			EndDialog(m_hwnd, IDOK);
			break;

		case ACTIVATION_OBSOLETELICENSE:
			EndDialog(m_hwnd, IDOK);
			UpgradeDialog dlg(m_hwnd);
			dlg.DoModal();
			break;
		}*/
    }
}

void LicenseManagerImpl::ShowUpgradeLicenseDialog()
{
    // create the demo dialog
    GtkWidget* pDialog = gtk_dialog_new_with_buttons(
        Zephyros::GetString(ZS_PREVVERSIONDLG_WNDTITLE).c_str(),
        NULL,
        GTK_DIALOG_MODAL,
        NULL
    );

    // create dialog widgets
    GtkWidget* pLabelTitel = gtk_label_new(Zephyros::GetString(ZS_PREVVERSIONDLG_TITLE).c_str());
    GtkWidget* pLabelDescription = gtk_label_new(Zephyros::GetString(ZS_PREVVERSIONDLG_DESCRIPTION).c_str());

    // add buttons
    const TCHAR* szUpgradeURL = static_cast<Zephyros::LicenseManager*>(Zephyros::GetLicenseManager())->GetUpgradeURL();
    if (szUpgradeURL != NULL && szUpgradeURL[0] != TCHAR('\0'))
        gtk_dialog_add_button(GTK_DIALOG(pDialog), Zephyros::GetString(ZS_PREVVERSIONDLG_UPGRADE).c_str(), GTK_RESPONSE_ACCEPT);
    gtk_dialog_add_button(GTK_DIALOG(pDialog), Zephyros::GetString(ZS_PREVVERSIONDLG_BACK).c_str(), GTK_RESPONSE_REJECT);

    // layout
    GtkWidget* pBoxContent = gtk_dialog_get_content_area(GTK_DIALOG(pDialog));
    gtk_box_pack_start(GTK_BOX(pBoxContent), pLabelTitel, FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(pBoxContent), pLabelDescription, FALSE, FALSE, 8);

    // display the dialog
    gtk_widget_show_all(pDialog);
    if (gtk_dialog_run(GTK_DIALOG(pDialog)) == GTK_RESPONSE_ACCEPT)
        OpenUpgradeLicenseURL();
}

void LicenseManagerImpl::OpenPurchaseLicenseURL()
{
    if (m_config.shopURL)
    {
        std::vector<Browser*>* pBrowsers = ((DefaultNativeExtensions*) Zephyros::GetNativeExtensions())->m_pBrowsers;
        BrowserUtil::OpenURLInBrowser(m_config.shopURL, BrowserUtil::GetDefaultBrowser(pBrowsers));
    }
}

void LicenseManagerImpl::OpenUpgradeLicenseURL()
{
    if (m_config.upgradeURL)
    {
        std::vector<Browser*>* pBrowsers = ((DefaultNativeExtensions*) Zephyros::GetNativeExtensions())->m_pBrowsers;
        BrowserUtil::OpenURLInBrowser(m_config.upgradeURL, BrowserUtil::GetDefaultBrowser(pBrowsers));
    }
}

bool LicenseManagerImpl::SendRequest(String url, std::string strPostData, std::stringstream& out)
{
	return false;
}

void LicenseManagerImpl::OnActivate(bool isSuccess)
{
    // close the demo dialog
}

void LicenseManagerImpl::OnReceiveDemoTokens(bool isSuccess)
{
	// nothing needs to be done here...
}

inline TCHAR toHex(TCHAR c)
{
    if (c > TCHAR('a'))
        return c - TCHAR('a') - 10;
    if (c > TCHAR('A'))
        return c - TCHAR('A') - 10;
    return c - TCHAR('0');
}

String LicenseManagerImpl::DecodeURI(String uri)
{
    StringStream ss;

    int nLen = uri.length();
    for (int i = 0; i < nLen; ++i)
    {
        TCHAR c = uri[i];

        if (c == TEXT('+'))
            ss << TEXT(' ');
        else if (c == TEXT('%') && i < nLen - 2 && isxdigit(uri[i + 1] && isxdigit(uri[i + 2])))
        {
            ss << (TCHAR) (16 * toHex(uri[i + 1]) + toHex(uri[i + 2]));
            i += 2;
        }
        else
            ss << c;
    }

    return ss.str();
}

} // namespace Zephyros
