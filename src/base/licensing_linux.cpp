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
#include <tchar.h>

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
	: m_timerId(-1), m_pDemoDlg(NULL)
{
	InitConfig();
}

LicenseManagerImpl::~LicenseManagerImpl()
{
    KillTimer();
}

bool LicenseManagerImpl::VerifyKey(String key, String info, const TCHAR* szPubkey)
{
    return false;
}


void LicenseManagerImpl::KillTimer()
{
}

void LicenseManagerImpl::ShowDemoDialog()
{
	// cancel any previous timer
	KillTimer();

	// show the demo dialog
	/*
	m_pDemoDlg = new DemoDialog(GetNumDaysLeft());
	INT_PTR result = m_pDemoDlg->DoModal();
	delete m_pDemoDlg;
	m_pDemoDlg = NULL;
	*/
	m_canStartApp = result == IDOK;

	// check the validity of the demo after 6 hours
	if (m_canStartApp && !IsActivated())
		;//m_timerId = SetTimer(NULL, 0, 6 * 3600 * 1000, DemoTimeout);

	// if the message loop is already running, post a quit message
	if (!m_canStartApp)
		App::QuitMessageLoop();
}

void LicenseManagerImpl::ShowEnterLicenseDialog()
{

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

} // namespace Zephyros
