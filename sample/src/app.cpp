#include "stdafx.h"
#include "Resource.h"


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




/*
#ifdef OS_WIN
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Resource.h"
#endif

#include "base/zephyros_impl.h"
#include "base/licensing.h"

#include "app.h"

//#include "path.h"

// TODO remove this somehow...
#if USE_CEF
#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"
#endif


FILE* _THE_FILE_;


int MAIN(MAIN_ARGS)
{
	char fn[100];
	sprintf_s(fn, "c:\\users\\ghost\\Temp\\%d.txt", GetCurrentProcessId());
	fopen_s(&_THE_FILE_, fn, "w+");
	fprintf(_THE_FILE_, "started...\n");fflush(_THE_FILE_);


#ifdef OS_WIN
	Zephyros::SetWindowsInfo(TEXT("Software\\Vanamco\\ZephyrosSampleApp"), IDI_ZEPHYROS_SAMPLE_APP, IDC_ZEPHYROS_SAMPLE_APP, IDC_ZEPHYROS_SAMPLE_APP);
#endif

	fprintf(_THE_FILE_, "creating license manager\n");fflush(_THE_FILE_);
    Zephyros::LicenseManager* pLicenseManager = new Zephyros::LicenseManager();
    pLicenseManager->SetLicenseInfo(1002, TEXT(""));
    pLicenseManager->AddObsoleteLicenseInfo(1001, TEXT(""));
    pLicenseManager->SetNumberOfDemoDays(7);
	fprintf(_THE_FILE_, "end config license manager\n");fflush(_THE_FILE_);
    
    //Zephyros::SetLicenseManager(pLicenseManager);
    
#ifdef OS_MACOSX
    Zephyros::SetUpdaterURL("http://awesome.vanamco.com/Ghostlab2/update/ghostlab2-cast.xml");
#endif
#ifdef OS_WIN
#ifdef _WIN64
    Zephyros::SetUpdaterURL(TEXT("http://awesome.vanamco.com/Ghostlab2/update/ghostlab2-cast-win64.xml"));
#else
    Zephyros::SetUpdaterURL(TEXT("http://awesome.vanamco.com/Ghostlab2/update/ghostlab2-cast-win32.xml"));
#endif
#endif
    
    //    Zephyros::SetNativeExtension();

}
*/