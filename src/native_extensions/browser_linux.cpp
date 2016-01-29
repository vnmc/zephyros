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

#include <iostream>
#include "native_extensions/browser.h"
#include "native_extensions/os_util.h"

#include "util/string_util.h"

#include "zephyros.h"


namespace Zephyros {
namespace BrowserUtil {

/**
 * Returns an array of all browsers available on the system.
 */
void FindBrowsers(std::vector<Browser*>** ppBrowsers)
{
	if (*ppBrowsers != NULL)
		return;

	*ppBrowsers = new std::vector<Browser*>();

    String strRet = OSUtil::Exec(TEXT("apropos \"web browser\""));
    std::vector<String> vecRet = Split(strRet, TEXT('\n'));
    for (String strLine : vecRet)
    {
        size_t pos = strLine.find(TEXT(' '));
        if (pos != String::npos)
        {
            String strBrowserName = strLine.substr(0, pos);

            String strWhichCmd = TEXT("which ");
            strWhichCmd.append(strBrowserName);
            String strPath = OSUtil::Exec(strWhichCmd);
            Trim(strPath);

            if (strPath.length() > 0)
                (*ppBrowsers)->push_back(new Browser(strBrowserName, TEXT(""), strPath, TEXT(""), false));
        }
    }
}

using namespace std;

bool OpenURLInBrowser(String url, Browser* browser)
{
    if (browser == NULL)
        return false;

    String cmdString = browser->GetIdentifier();
    cmdString.append(" ");
    cmdString.append(url);
    cmdString.append(" &");

    system( cmdString.c_str() );
    return true;
}


void CleanUp(std::vector<Browser*>* pBrowsers)
{
    for (Browser *pBrowser: *pBrowsers)
        delete pBrowser;
}

} // namespace BrowserUtil
} // namespace Zephyros
