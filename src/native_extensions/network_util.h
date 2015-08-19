//
//  NetworkUtil.h
//  Ghostlab
//
//  Created by Matthias Christen on 06.09.13.
//
//

#include <vector>
#include "base/types.h"


namespace Zephyros {
namespace NetworkUtil {

JavaScript::Array GetNetworkIPs();

String GetPrimaryMACAddress();
    
std::vector<String> GetAllMACAddresses();
    
bool GetProxyForURL(String url, String& proxyType, String& host, int& port, String& username, String& password);

#ifdef USE_WEBVIEW
// For Mac/WebView: Make a HTTP request
void MakeRequest(JSObjectRef callback, String httpMethod, String url, String postData, String postDataContentType, String responseDataType);
#endif

} // namespace NetworkUtil
} // namespace Zephyros
