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

#include <gtk/gtk.h>

#include "zephyros_strings.h"

#include "base/app.h"
#include "base/licensing.h"
#include "base/cef/client_handler.h"

#include "util/string_util.h"
#include "native_extensions/file_util.h"


extern CefRefPtr<Zephyros::ClientHandler> g_handler;


namespace Zephyros {


//////////////////////////////////////////////////////////////////////////
// LicenseData Implementation

LicenseData::LicenseData(const TCHAR* szLicenseInformationFilename)
	: m_szLicenseInfoFilename(szLicenseInformationFilename)
{
	// read the license data from the file


}

void LicenseData::Save()
{
	// save the license data to the file


}

String LicenseData::Now()
{
	String result = TEXT("");

	return result;
}


//////////////////////////////////////////////////////////////////////////
// LicenseManager Implementation

LicenseManagerImpl::LicenseManagerImpl()
//	: m_timerId(-1), m_pDemoDlg(NULL)
{
	InitConfig();
}

LicenseManagerImpl::~LicenseManagerImpl()
{
//    KillTimer();
}

bool LicenseManagerImpl::VerifyKey(String key, String info, const TCHAR* szPubkey)
{
    return false;
}

/*
// TODO: add if needed
void LicenseManagerImpl::KillTimer()
{
}
*/

void LicenseManagerImpl::ShowDemoDialog()
{
	// cancel any previous timer
//	KillTimer();

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

   /* Add the label, and show everything we've added to the dialog. */

    gtk_widget_show_all(pDialog);

   //gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),label);

    // show the demo dialog
    m_canStartApp = gtk_dialog_run(GTK_DIALOG(pDialog)) == 1 && ContinueDemo();

	// check the validity of the demo after 6 hours
	if (m_canStartApp && !IsActivated())
		;//m_timerId = SetTimer(NULL, 0, 6 * 3600 * 1000, DemoTimeout);

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
    GtkWidget* pLabelName = gtk_label_new(Zephyros::GetString(ZS_DEMODLG_REMAINING_TIME_DESC).c_str());
    GtkWidget* pTextName = gtk_entry_new();
    GtkWidget* pLabelOrganization = gtk_label_new(Zephyros::GetString(ZS_ENTERLICKEYDLG_ORGANIZATION).c_str());
    GtkWidget* pTextOrganization = gtk_entry_new();
    GtkWidget* pLabelLicenseKey = gtk_label_new(Zephyros::GetString(ZS_ENTERLICKEYDLG_LICKEY).c_str());
    GtkWidget* pTextLicenseKey = gtk_entry_new();

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
    if (gtk_dialog_run(GTK_DIALOG(pDialog)) == GTK_RESPONSE_ACCEPT)
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

}

void LicenseManagerImpl::OpenUpgradeLicenseURL()
{
}

bool LicenseManagerImpl::SendRequest(String url, std::string strPostData, std::stringstream& out)
{
	return false;
}

void LicenseManagerImpl::OnActivate(bool isSuccess)
{
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
