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

#include "stdafx.h"
#include "Resource.h"

#else

#ifdef USE_CEF
#include "lib/cef/include/cef_base.h"
#include "lib/cef/include/base/cef_lock.h"
#include "lib/cef/include/cef_client.h"
#include "lib/cef/include/wrapper/cef_helpers.h"
#endif

#include "zephyros.h"
#include "native_extensions.h"

#endif


#ifdef OS_MACOSX
#define UPDATER_URL TEXT("http://my-server.com/zephyros-sampleapp-cast.xml")
#endif
#ifdef OS_WIN
#ifdef _WIN64
#define UPDATER_URL TEXT("http://my-server.com/zephyros-sampleapp-cast-win64.xml")
#else
#define UPDATER_URL TEXT("http://my-server.com/zephyros-sampleapp-cast-win32.xml")
#endif
#endif


class MyNativeExtensions : public Zephyros::DefaultNativeExtensions
{
	virtual void AddNativeExtensions(Zephyros::NativeJavaScriptFunctionAdder* extensionHandler)
	{
		// add the default native extensions
		DefaultNativeExtensions::AddNativeExtensions(extensionHandler);

		// example: add a new native function
		// fibonacci: (n: number, callback(result: number)) => void
		extensionHandler->AddNativeJavaScriptFunction(
			TEXT("fibonacci"),
			FUNC({
				int n = args->GetInt(0);
				long a = 0;
				long b = 1;
				
				for (int i = 0; i < n; i++)
				{
					long c = a + b;
					a = b;
					b = c;
				}

				ret->SetDouble(0, b);
				return NO_ERROR;
			},
			ARG(VTYPE_INT, "n")
		));
	}
};


int MAIN(MAIN_ARGS)
{
	/*
	Zephyros::LicenseManager* pLicenseManager = new Zephyros::LicenseManager();
    pLicenseManager->SetLicenseInfo(102, TEXT(""));
    pLicenseManager->AddObsoleteLicenseInfo(101, TEXT(""));
    pLicenseManager->SetNumberOfDemoDays(7);

	Zephyros::SetLicenseManager(pLicenseManager);
	//*/

	Zephyros::SetNativeExtensions(new MyNativeExtensions());

	//Zephyros::SetUpdaterURL(UPDATER_URL);

#ifdef OS_WIN
	Zephyros::SetWindowsInfo(TEXT("Software\\Vanamco\\ZephyrosSampleApp"), IDI_ZEPHYROS_SAMPLEAPP, IDC_ZEPHYROS_SAMPLEAPP, IDC_ZEPHYROS_SAMPLEAPP);
	#include "..\res\windows\content.cpp"
#endif

	return Zephyros::Run(INIT_APPLICATION_ARGS, TEXT("Zephyros Sample App"), TEXT("1.0.0"), TEXT("app/index.html"));
}
