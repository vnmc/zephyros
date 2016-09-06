/*******************************************************************************
 * Copyright (c) 2015-2016 Vanamco AG, http://www.vanamco.com
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


#include <set>
#include <iomanip>

#include <ifaddrs.h>
#include <netdb.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "base/app.h"
#include "base/cef/client_handler.h"

#include "native_extensions/network_util.h"
#include "native_extensions/os_util.h"
#include "util/string_util.h"

#include "lib/cef/include/cef_parser.h"

#include <iostream>


namespace Zephyros {
namespace NetworkUtil {

/**
 * Returns an array of network IP addresses.
 */
std::vector<String> GetNetworkIPs()
{
    std::vector<String> addrs;

    struct ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) != -1)
    {
        int i = 0;
        char host[NI_MAXHOST];

        for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
                continue;

            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0)
                addrs.push_back(String(host));
        }

        freeifaddrs(ifaddr);
    }

    return addrs;
}

String GetPrimaryMACAddress()
{
    std::vector<String> addresses = GetAllMACAddresses();
	return addresses.at(0);
}

String GetMACAddressForIFName(String strIFName)
{
    struct ifreq s;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    String strRet;

    strcpy(s.ifr_name, strIFName.c_str());
    if (ioctl(fd, SIOCGIFHWADDR, &s) == 0)
    {
        StringStream ss;
		ss << std::hex << std::setfill(TEXT('0'));
        for (int i = 0; i < 6; ++i)
        {
            if (i > 0)
                ss << TEXT(":");
            ss << std::setw(2) << (int) (unsigned char) s.ifr_addr.sa_data[i];
        }

        strRet = ss.str();
    }
    else
        strRet = TEXT("00:00:00:00:00:00");

    close(fd);

    return strRet;
}

std::vector<String> GetAllMACAddresses()
{
	std::vector<String> vecResult;

    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) != -1)
    {
        std::set<String> names;
        for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
            names.insert(ifa->ifa_name);

        for (String name : names)
        {
            String strMACAddr = GetMACAddressForIFName(name);
            if (strMACAddr != TEXT("00:00:00:00:00:00"))
                vecResult.push_back(strMACAddr);
        }

        freeifaddrs(ifaddr);
    }

    if (vecResult.empty())
        vecResult.push_back(TEXT("00:00:00:00:00:00"));

	return vecResult;
}

bool GetProxyForURL(String url, String& proxyType, String& host, int& port, String& username, String& password)
{
    proxyType = TEXT("");
	host = TEXT("");
	port = -1;
	username = TEXT("");
	password = TEXT("");

    String urlHost = TEXT("");
    if (url.length() > 0)
    {
        CefURLParts urlParts;
        CefParseURL(url, urlParts);
        urlHost = CefString(&urlParts.host).ToString();
    }

    char* proxyEnv = NULL;
    char* noProxy = getenv(TEXT("NO_PROXY"));
    char* digit = getenv(TEXT("PATH"));
    String myProxyType = TEXT("http");

    if (noProxy != NULL)
    {
        std::vector<String> proxyExceptions = Split(String(noProxy), TEXT(','));
        for (String item : proxyExceptions)
        {
            if (urlHost.compare(item) == 0)
                return false;
            // TODO: check for .domain.tld, ip/MASK
        }
    }

    if (url.find(TEXT("http://")) == 0)
    {
        proxyEnv = getenv(TEXT("HTTP_PROXY"));
        myProxyType = TEXT("http");
    }
    else if (url.find(TEXT("https://")) == 0)
    {
        proxyEnv = getenv(TEXT("HTTPS_PROXY"));
        myProxyType = TEXT("https");
    }

    if (proxyEnv == NULL)
        return false;

    CefURLParts proxyUrlParts;
    CefParseURL(String(proxyEnv), proxyUrlParts);
    host = CefString(&proxyUrlParts.host).ToString();
    port = atoi(CefString(&proxyUrlParts.port).ToString().c_str());
    username = CefString(&proxyUrlParts.username).ToString();
    password = CefString(&proxyUrlParts.password).ToString();
    proxyType = myProxyType;

	return true;
}

} // namespace NetworkUtil
} // namespace Zephyros
