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


#include "stdafx.h"

#ifdef OS_WIN
#include "Resource.h"
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
#ifndef APPSTORE
    //*
	Zephyros::LicenseManager* pLicenseManager = new Zephyros::LicenseManager();
    pLicenseManager->SetLicenseInfo(102, TEXT(
        "MIIBtzCCASwGByqGSM44BAEwggEfAoGBAJ0BnatI/iQGeWpD7lIepUJVDogTj3vn\n"
        "0eJkQ49JPm+7w8H7YaNse6nBIOdbKhbkaNA+l5kDeByXUZUDcgov2P+z5LjCNB0w\n"
        "bSVw4muqG4e+WRLJEY7Sgw2F5ng14rd8GOtELciQu8Zd+jfamp/iuD1ad8P4NgNd\n"
        "kBlVdI/t+FO7AhUAt98S5RGrSCf3gBmZ4nfS2aW6s30CgYEAnBGa/K067i6oErwb\n"
        "/7nW8P7Gna0VRHYl9BAur0YNrNN9E7TyJSNZYbEIcGacb7DUzkZYqkCCuQR5PVlQ\n"
        "V9cQz6kH2bhElWAYzg61TCnoCE9gI6ZEyGdHrAVoLtm3rW3c+IpxGC0nEyXyCmQR\n"
        "465bjiSGGESEU1S/o1yntcLZNRQDgYQAAoGAZrOFo0mgo0axU9KaUSCBCNOC2QNE\n"
        "m8w/7KldFaaLdJIEH9CNkGnjkYKvDDqM/Ol+7BzopgiG9vnNtQcJjtsw7ANF4kxe\n"
        "NVNR/AvSQ8YEyXPIkyd+RyoJlJsLyAFyySMXlHvQldp8iu0lKbDVlqqXZ9m0N3Is\n"
        "xr2dhgzBMlgdzIg="));
    //pLicenseManager->AddObsoleteLicenseInfo(101, TEXT(""));
    pLicenseManager->SetNumberOfDemoDays(7);
    //pLicenseManager->SetAPIURLs(TEXT("demo"), TEXT("activation"), TEXT("deactivation"));
    pLicenseManager->SetAPIURLs(
        TEXT("http://ghostlab2.vanamco.com/zephyros-demo/generate-demotokens.php"),
        TEXT("http://ghostlab2.vanamco.com/zephyros-demo/generate-activation.php"),
        TEXT("http://ghostlab2.vanamco.com/zephyros-demo/generate-deactivation.php")
    );

	Zephyros::SetLicenseManager(pLicenseManager);
	//*/
#endif

	Zephyros::SetNativeExtensions(new MyNativeExtensions());

	//Zephyros::SetUpdaterURL(UPDATER_URL);

    //Zephyros::UseLogging(true);

#ifdef OS_WIN
	Zephyros::SetWindowsInfo(TEXT("Software\\Vanamco\\ZephyrosSampleApp"), IDI_ZEPHYROS_SAMPLEAPP, IDC_ZEPHYROS_SAMPLEAPP, IDC_ZEPHYROS_SAMPLEAPP);
	#include "..\res\windows\content.cpp"
#endif

#ifdef OS_MACOSX
    Zephyros::SetOSXInfo(TEXT("Localizable"));
#endif

	return Zephyros::Run(INIT_APPLICATION_ARGS, TEXT("Zephyros Sample App"), TEXT("1.0.0"), TEXT("app/index.html"));
}
