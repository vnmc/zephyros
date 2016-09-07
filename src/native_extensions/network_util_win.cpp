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


// hack to make regex compile
#pragma warning(disable: 4005)
#define _HAS_EXCEPTIONS 1

#include <set>
#include <iomanip>
#include <algorithm>

#include <tchar.h>
#include <WinSock2.h>
#include <IPHlpApi.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#include <winhttp.h>
#include <regex>

#include "base/app.h"
#include "base/cef/client_handler.h"

#include "native_extensions/network_util.h"


#define MAX_BUF_LEN 128


namespace Zephyros {
namespace NetworkUtil {

/**
 * Returns the IP address (v4 or v6) from a SOCKADDR pointer.
 * NOTE: InetNtoP exists as of Windows Vista
 */
String GetIPAddress(LPSOCKADDR pAddr)
{
	TCHAR buf[MAX_BUF_LEN];

	switch (pAddr->sa_family)
	{
	case AF_INET:
		InetNtop(AF_INET, &(((sockaddr_in*) pAddr)->sin_addr), buf, MAX_BUF_LEN);
		break;

	case AF_INET6:
		InetNtop(AF_INET6, &(((sockaddr_in6*) pAddr)->sin6_addr), buf, 64);
		break;

	default:
		return String(TEXT(""));
	}

	return String(buf);
}

/**
 * Returns an array of network IP addresses.
 */
std::vector<String> GetNetworkIPs()
{
	std::set<String> setAddrs;
    JavaScript::Array addrs = JavaScript::CreateArray();

	// we only want IPv4 unicast addresses
	ULONG family = AF_INET;
	ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME;

	ULONG size = 0;
	GetAdaptersAddresses(family, flags, NULL, NULL, &size);
	if (size > 0)
	{
		BYTE* pBuf = new BYTE[size];
		if (GetAdaptersAddresses(family, flags, NULL, (PIP_ADAPTER_ADDRESSES) pBuf, &size) == ERROR_SUCCESS)
		{
			for (PIP_ADAPTER_ADDRESSES pAddress = (PIP_ADAPTER_ADDRESSES) pBuf; pAddress; pAddress = pAddress->Next)
			{
				// filter out non-ethernet adapters
				if (pAddress->IfType != IF_TYPE_ETHERNET_3MBIT && pAddress->IfType != IF_TYPE_ETHERNET_CSMACD && pAddress->IfType != IF_TYPE_IEEE80211)
					continue;

				// filter out disabled adapters
				if (pAddress->OperStatus != IF_OPER_STATUS::IfOperStatusUp && pAddress->OperStatus != IF_OPER_STATUS::IfOperStatusDormant)
					continue;

				for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAddress->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next)
				{
					String ip = GetIPAddress(pUnicast->Address.lpSockaddr);
					if (ip.length() > 0)
						setAddrs.insert(ip);
				}

				for (PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = pAddress->FirstMulticastAddress; pMulticast; pMulticast = pMulticast->Next)
				{
					String ip = GetIPAddress(pMulticast->Address.lpSockaddr);
					if (ip.length() > 0)
						setAddrs.insert(ip);
				}

				for (PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = pAddress->FirstAnycastAddress; pAnycast; pAnycast = pAnycast->Next)
				{
					String ip = GetIPAddress(pAnycast->Address.lpSockaddr);
					if (ip.length() > 0)
						setAddrs.insert(ip);
				}
			}
		}

		delete[] pBuf;
	}

	// add the IP addresses to the result list
	std::vector<String> vecAddrs;
	for (String ip : setAddrs)
		vecAddrs.push_back(ip);

    return vecAddrs;
}

String GetPrimaryMACAddress()
{
	// the primary MAC address is the MAC address of the adapter with the lowest index
	// (http://www.codeproject.com/Articles/13421/Getting-the-Physical-MAC-address-of-a-Network-Inte)
	// i.e., the first one returned by GetAllMACAddresses() (since the vector is sorted by index)

	std::vector<String> addresses = GetAllMACAddresses();
	return addresses.at(0);
}

std::vector<String> GetAllMACAddresses()
{
	std::vector<PIP_ADAPTER_ADDRESSES> vecAdapters;
	std::vector<String> vecResult;
	BYTE* pBuf = NULL;

	ULONG size = 0;
	GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_FRIENDLY_NAME, NULL, NULL, &size);
	if (size > 0)
	{
		pBuf = new BYTE[size];
		if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_FRIENDLY_NAME, NULL, (PIP_ADAPTER_ADDRESSES) pBuf, &size) == ERROR_SUCCESS)
		{
			for (PIP_ADAPTER_ADDRESSES pAddress = (PIP_ADAPTER_ADDRESSES) pBuf; pAddress; pAddress = pAddress->Next)
			{
				// filter out non-ethernet adapters
				if (pAddress->IfType != IF_TYPE_ETHERNET_3MBIT && pAddress->IfType != IF_TYPE_ETHERNET_CSMACD && pAddress->IfType != IF_TYPE_IEEE80211)
					continue;
				vecAdapters.push_back(pAddress);
			}
		}
	}

	// return a default address if no primary adapter could be found
	if (vecAdapters.empty())
		vecResult.push_back(TEXT("00:00:00:00:00:00"));
	else
	{
		// sort addresses by index so that the primary adapter comes first
		// (the primary MAC address is the MAC address of the adapter with the lowest index, cf.
		// http://www.codeproject.com/Articles/13421/Getting-the-Physical-MAC-address-of-a-Network-Inte)
		std::sort(
			vecAdapters.begin(),
			vecAdapters.end(),
			[](const PIP_ADAPTER_ADDRESSES& a1, const PIP_ADAPTER_ADDRESSES& a2) -> bool
			{
				return a1->IfIndex < a2->IfIndex;
			}
		);

		for (PIP_ADAPTER_ADDRESSES pAdapter : vecAdapters)
		{
			StringStream ss;
			ss << std::hex << std::setfill(TEXT('0'));
			for (int i = 0; i < (int) pAdapter->PhysicalAddressLength; ++i)
			{
				if (i > 0)
					ss << TEXT(":");
				ss << std::setw(2) << pAdapter->PhysicalAddress[i];
			}

			vecResult.push_back(ss.str());
		}
	}

	if (pBuf != NULL)
		delete[] pBuf;

	return vecResult;
}

/**
 * Parse the proxy info in proxy.
 * It is a string of the format ([<scheme>=][<scheme>"://"]<server>[":"<port>])[;(...)].
 */
void ParseProxyInfo(LPTSTR proxy, String& proxyType, String& host, int& port, String& username, String& password)
{
	StringStream ssProxy(proxy);
	while (!ssProxy.eof())
	{
		// split the string at semi-colons
		String strProxy;
		std::getline(ssProxy, strProxy, TEXT(';'));

		// match the parts against the pattern
		std::wsmatch matches;
		if (std::regex_match(strProxy, matches, std::wregex(TEXT("(?:([\\w\\d]+)=)?(?:([\\w\\d]+)://)?([\\d\\.]+)(?::([\\d]+))?"))))
		{
			proxyType = matches[1].matched ? matches[1].str() : (matches[2].matched ? matches[2].str() : TEXT("http"));

			if (matches[3].matched)
				host = matches[3].str();

			if (matches[4].matched)
				port = _tstoi(matches[4].str().c_str());
		}

		// return the first non-FTP proxy
		if (proxyType != TEXT("ftp"))
			break;
	}
}

bool GetProxyForURL(String url, String& proxyType, String& host, int& port, String& username, String& password)
{
	bool ret = false;

	proxyType = TEXT("");
	host = TEXT("");
	port = -1;
	username = TEXT("");
	password = TEXT("");

	HINTERNET hSession = WinHttpOpen(Zephyros::GetAppName(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession != NULL)
	{
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
				if (WinHttpGetProxyForUrl(hSession, url.c_str(), &autoproxyOption, &proxyInfo))
				{
					if (proxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NAMED_PROXY && proxyInfo.lpszProxy != NULL)
					{
						ret = true;
						ParseProxyInfo(proxyInfo.lpszProxy, proxyType, host, port, username, password);
					}
				}
			}
			
			if (ieProxyConfig.lpszProxy != NULL && !ret)
			{
				ret = true;
				ParseProxyInfo(ieProxyConfig.lpszProxy, proxyType, host, port, username, password);
			}
			else
			{
				ret = true;
				proxyType = TEXT("none");
			}
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

		WinHttpCloseHandle(hSession);
	}
	
	return ret;
}

} // namespace NetworkUtil
} // namespace Zephyros
