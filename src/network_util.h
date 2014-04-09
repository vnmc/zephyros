//
//  NetworkUtil.h
//  Zephyros
//
//  Created by Matthias Christen on 06.09.13.
//
//

#include "types.h"


namespace NetworkUtil {

#ifdef USE_WEBVIEW
// For Mac/WebView: Make a HTTP request
void MakeRequest(JSObjectRef callback, String httpMethod, String url, String postData, String postDataContentType, String responseDataType);
#endif

} // namespace NetworkUtil
