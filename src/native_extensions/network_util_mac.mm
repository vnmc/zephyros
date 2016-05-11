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


#import <SystemConfiguration/SystemConfiguration.h>
#import <arpa/inet.h>
#import <ifaddrs.h>
#import <net/if.h>

#import "base/logging.h"

#import "native_extensions/network_util.h"

#import "util/GetPrimaryMACAddress.h"
#import "util/NSData+Base64.h"
#import "util/EMKeychainItem.h"


#ifdef USE_WEBVIEW
extern JSContextRef g_ctx;

@interface ConnectionDelegate : NSObject <NSURLConnectionDelegate>

@property NSMutableData *data;
@property JSObjectRef callback;
@property NSString *responseDataType;
@property NSString *responseMimeType;

@end


@implementation ConnectionDelegate

- (id) init: (JSObjectRef) callback withResponseDataType: (NSString*) type
{
    self = [super init];
    
    _data = [[NSMutableData alloc] init];
    _callback = callback;
    _responseDataType = type;
    
    return self;
}

- (void) connection: (NSURLConnection*) connection didReceiveResponse: (NSURLResponse*) response
{
    _data.length = 0;
    _responseMimeType = response.MIMEType;
}

- (void) connection: (NSURLConnection*) connection didReceiveData: (NSData*) data
{
    [_data appendData: data];
}

- (void) connectionDidFinishLoading: (NSURLConnection*) connection
{
    if (JSObjectIsFunction(g_ctx, _callback))
    {
        NSString *strRetData = nil;
        if ([_responseDataType isEqualToString: @"base64"])
            strRetData = [_data base64EncodedString];
        else
            strRetData = [[NSString alloc] initWithData: _data encoding: NSUTF8StringEncoding];
    
        if (strRetData == nil)
            strRetData = [[NSString alloc] initWithData: _data encoding: NSASCIIStringEncoding];
    
        if (strRetData == nil)
            return;
    
        JSStringRef retData = JSStringCreateWithCFString((__bridge CFStringRef) strRetData);
        JSStringRef retContentType = JSStringCreateWithCFString((__bridge CFStringRef) _responseMimeType);
    
        JSValueRef args[3];
        args[0] = JSValueMakeString(g_ctx, retData);
        args[1] = JSValueMakeString(g_ctx, retContentType);
        args[2] = JSValueMakeBoolean(g_ctx, true);
    
        JSObjectCallAsFunction(g_ctx, _callback, NULL, 3, args, NULL);
    
        JSStringRelease(retData);
        JSStringRelease(retContentType);
    }
    
    JSValueUnprotect(g_ctx, _callback);
}

- (void) connection: (NSURLConnection*) connection didFailWithError: (NSError*) error
{
    JSStringRef empty = JSStringCreateWithUTF8CString("");
    
    JSValueRef args[3];
    args[0] = JSValueMakeString(g_ctx, empty);
    args[1] = JSValueMakeString(g_ctx, empty);
    args[2] = JSValueMakeBoolean(g_ctx, false);
    
    JSObjectCallAsFunction(g_ctx, _callback, NULL, 3, args, NULL);
    
    JSStringRelease(empty);
    JSValueUnprotect(g_ctx, _callback);
}

@end
#endif


namespace Zephyros {
namespace NetworkUtil {

//
// Returns an array of network IP addresses.
// http://stackoverflow.com/a/14697468
//
JavaScript::Array GetNetworkIPs()
{
    JavaScript::Array addrs = JavaScript::CreateArray();
    int idx = 0;
    
    addrs->SetString(idx++, "127.0.0.1");
    
    struct ifaddrs *interfaces;
    if (getifaddrs(&interfaces) == 0)
    {
        struct ifaddrs *interface;
        
        for (interface = interfaces; interface != NULL; interface = interface->ifa_next)
        {
            if ((interface->ifa_flags & IFF_UP) && !(interface->ifa_flags & IFF_LOOPBACK))
            {
                const struct sockaddr_in *addr = (const struct sockaddr_in*) interface->ifa_addr;
                
                if (addr && (addr->sin_family == PF_INET /*|| addr->sin_family == PF_INET6*/))
                {
                    char ipAddress[INET6_ADDRSTRLEN];
                    inet_ntop(addr->sin_family, &(addr->sin_addr), ipAddress, INET6_ADDRSTRLEN);
                    addrs->SetString(idx++, ipAddress);
                }
            }
        }
        
        freeifaddrs(interfaces);
    }
    
    return addrs;
}
    
String GetPrimaryMACAddress()
{
    std::vector<String> addresses = getMACAddresses(true);
    return addresses.at(0);
}
    

std::vector<String> GetAllMACAddresses()
{
    return getMACAddresses(false);
}

/**
 * Callback for CFNetworkExecuteProxyAutoConfigurationURL. client is a
 * pointer to a CFTypeRef. This stashes either error or proxies in that location.
 */
static void ResultCallback(void * client, CFArrayRef proxies, CFErrorRef error)
{
    CFTypeRef* resultPtr = (CFTypeRef*) client;
        
    if (error != NULL)
        *resultPtr = CFRetain(error);
    else
        *resultPtr = CFRetain(proxies);

    CFRunLoopStop(CFRunLoopGetCurrent());
}

/**
 * Finds the system proxy for the URL url.
 * Inspired by http://electricsheep.googlecode.com/svn-history/r33/trunk/client_generic/MacBuild/proximus.c
 */
bool GetProxyForURL(String url, String& type, String& host, int& port, String& username, String& password)
{
    DEBUG_LOG(@"GetProxyForURL %s", url.c_str());
    
    bool ret = false;
    CFTypeRef res = NULL;

    NSURL *pUrl = [NSURL URLWithString: [NSString stringWithUTF8String: url.c_str()]];
        
    CFDictionaryRef proxySettings = SCDynamicStoreCopyProxies(NULL);
    if (proxySettings != NULL)
    {
        DEBUG_LOG(@"GetProxyForURL has proxy settings");
        
        CFArrayRef proxies = CFNetworkCopyProxiesForURL((__bridge CFURLRef) pUrl, proxySettings);
        if (proxies != NULL && CFArrayGetCount(proxies) > 0)
        {
            DEBUG_LOG(@"GetProxyForURL has proxies");
            
            CFDictionaryRef bestProxy = (CFDictionaryRef) CFArrayGetValueAtIndex(proxies, 0);
            if (bestProxy != NULL)
            {
                DEBUG_LOG(@"GetProxyForURL has best proxy");
                
                CFStringRef proxyType = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyTypeKey);
                
                // get the proxy from the autoconfig
                if (proxyType != NULL && !CFEqual(proxyType, kCFProxyTypeNone) && CFEqual(proxyType, kCFProxyTypeAutoConfigurationURL))
                {
                    DEBUG_LOG(@"GetProxyForURL autoconfig, proxyType=%@", proxyType);
                    
                    CFURLRef scriptURL = (CFURLRef) CFDictionaryGetValue(bestProxy, kCFProxyAutoConfigurationURLKey);
                    if (scriptURL != NULL)
                    {
                        DEBUG_LOG(@"GetProxyForURL has script URL %@", scriptURL);
                        CFStreamClientContext context = { 0, &res, NULL, NULL, NULL };
                            
                        // Work around <rdar://problem/5530166>.  This dummy call to
                        // CFNetworkCopyProxiesForURL initialise some state within CFNetwork
                        // that is required by CFNetworkCopyProxiesForAutoConfigurationScript.
                        CFDictionaryRef settings = CFDictionaryCreate(NULL, NULL, NULL, 0, NULL, NULL);
                        CFNetworkCopyProxiesForURL((__bridge CFURLRef) pUrl, settings);
                            
                        CFRunLoopSourceRef rls = CFNetworkExecuteProxyAutoConfigurationURL(scriptURL, (__bridge CFURLRef) pUrl, ResultCallback, &context);
                        if (rls != NULL)
                        {
                            DEBUG_LOG(@"GetProxyForURL has run loop source");
                            
                            CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
                            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0e10, false);
                            CFRunLoopRemoveSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
                            
                            DEBUG_LOG(@"GetProxyForURL releaseing run loop source");
                            CFRelease(rls);
                            
                            if (res && CFGetTypeID(res) == CFArrayGetTypeID())
                            {
                                DEBUG_LOG(@"GetProxyForURL has res");
                                
                                bestProxy = (CFDictionaryRef) CFArrayGetValueAtIndex((CFArrayRef) res, 0);
                                if (bestProxy != NULL)
                                {
                                    DEBUG_LOG(@"GetProxyForURL has best proxy (2)");
                                    proxyType = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyTypeKey);
                                    DEBUG_LOG(@"GetProxyForURL proxyType=%@", proxyType);
                                }
                            }
                        }
                    }
                }
                
                // parse the proxy information
                if (bestProxy != NULL && proxyType != NULL)
                {
                    DEBUG_LOG(@"GetProxyForURL has bestProxy and proxyType");
                    
                    ret = true;
                    
                    if (CFEqual(proxyType, kCFProxyTypeNone))
                        type = "none";
                    else if (CFEqual(proxyType, kCFProxyTypeHTTP))
                        type = "http";
                    else if (CFEqual(proxyType, kCFProxyTypeHTTPS))
                        type = "https";
                    else if (CFEqual(proxyType, kCFProxyTypeSOCKS))
                        type = "socks";
                    else
                        type = "unknown";
                    
                    DEBUG_LOG(@"GetProxyForURL set type=%s", type.c_str());

                    host = "";
                    port = -1;
                    username = "";
                    password = "";

                    char buf[512];

                    CFStringRef hostStr = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyHostNameKey);
                    if (hostStr != NULL)
                    {
                        DEBUG_LOG(@"GetProxyForURL has hostStr");
                        
                        if (CFStringGetCString(hostStr, buf, sizeof(buf) - 1, kCFStringEncodingUTF8))
                        {
                            host = String(buf);
                            DEBUG_LOG(@"GetProxyForURL set host=%s", host.c_str());
                        }

                        if (host.length() > 0)
                        {
                            DEBUG_LOG(@"GetProxyForURL get port");
                            CFNumberRef portNum = (CFNumberRef) CFDictionaryGetValue(bestProxy, kCFProxyPortNumberKey);
                            if (portNum != NULL)
                            {
                                DEBUG_LOG(@"GetProxyForURL has portNum");
                            
                                CFNumberGetValue(portNum, kCFNumberIntType, &port);
                                DEBUG_LOG(@"GetProxyForURL set port=%d", port);
                            }
                        
                            DEBUG_LOG(@"GetProxyForURL get port");
                            CFStringRef userStr = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyUsernameKey);
                            if (userStr != NULL)
                            {
                                DEBUG_LOG(@"GetProxyForURL has userStr");
                            
                                if (CFStringGetCString(userStr, buf, sizeof(buf) - 1, kCFStringEncodingUTF8))
                                {
                                    username = String(buf);
                                    DEBUG_LOG(@"GetProxyForURL set username=%s", username.c_str());
                                }
                            }
                        
                            DEBUG_LOG(@"GetProxyForURL get port");
                            CFStringRef passwordStr = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyPasswordKey);
                            if (passwordStr != NULL)
                            {
                                DEBUG_LOG(@"GetProxyForURL has passwordStr");
                            
                                if (CFStringGetCString(passwordStr, buf, sizeof(buf) - 1, kCFStringEncodingUTF8))
                                {
                                    password = String(buf);
                                    DEBUG_LOG(@"GetProxyForURL set password=%s", password.c_str());
                                }
                            }
                        
                            // if no user name / password is in the dictionary, check the keychain
                            DEBUG_LOG(@"GetProxyForURL get username/password from key chain?");
                            if (userStr == NULL || passwordStr == NULL)
                            {
                                DEBUG_LOG(@"GetProxyForURL has host but no user/password");
                            
                                EMInternetKeychainItem *keychainItem = [EMInternetKeychainItem internetKeychainItemForServer: [NSString stringWithUTF8String: host.c_str()]
                                                                                                                withUsername: @""
                                                                                                                        path: @""
                                                                                                                        port: port
                                                                                                                    protocol: kSecProtocolTypeAny];
                                DEBUG_LOG(@"GetProxyForURL retrieved keychain item");
                            
                                if (keychainItem != nil)
                                {
                                    DEBUG_LOG(@"GetProxyForURL has keychain item");
                                
                                    username = String([keychainItem.username UTF8String]);
                                    password = String([keychainItem.password UTF8String]);
                                
                                    DEBUG_LOG(@"GetProxyForURL from keychain item: username=%s, password=%s", username.c_str(), password.c_str());
                                }
                            }
                        }
                        else
                            type = "none";
                    }
                    else
                        type = "none";
                }
            }

            DEBUG_LOG(@"GetProxyForURL releasing proxies");
            CFRelease(proxies);
        }
        else if (proxies != NULL && CFArrayGetCount(proxies) == 0)
        {
            DEBUG_LOG(@"GetProxyForURL no proxies");
            ret = true;
        }
        
        DEBUG_LOG(@"GetProxyForURL releasing proxySettings");
        CFRelease(proxySettings);
    }
 
    if (res != NULL)
    {
        DEBUG_LOG(@"GetProxyForURL releasing res");
        CFRelease(res);
    }

    return ret;
}

void SetIELoopbackExemption(bool bSetLoopbackExempt)
{
    // not used on Mac
}

    
#ifdef USE_WEBVIEW
    
void MakeRequest(JSObjectRef callback, String httpMethod, String url, String postData, String postDataContentType, String responseDataType)
{
    JSValueProtect(g_ctx, callback);
    
    NSMutableURLRequest *urlRequest = [NSMutableURLRequest requestWithURL: [NSURL URLWithString: [NSString stringWithUTF8String: url.c_str()]]];
    urlRequest.HTTPMethod = [NSString stringWithUTF8String: httpMethod.c_str()];
    
    if (postData != "")
    {
        urlRequest.HTTPBody = [[NSString stringWithUTF8String: postData.c_str()] dataUsingEncoding: NSUTF8StringEncoding];

        if (postDataContentType != "")
            [urlRequest setValue: [NSString stringWithUTF8String: postDataContentType.c_str()] forHTTPHeaderField: @"Content-Type"];
        else
            [urlRequest setValue: @"application/x-www-form-urlencoded" forHTTPHeaderField: @"Content-Type"];

        [urlRequest setValue: [NSString stringWithFormat: @"%ld", urlRequest.HTTPBody.length] forHTTPHeaderField: @"Content-Length"];
    }
    
    NSURLConnection *conn = [[NSURLConnection alloc] initWithRequest: urlRequest
                                                            delegate: [[ConnectionDelegate alloc] init: callback withResponseDataType: [NSString stringWithUTF8String: responseDataType.c_str()]]];
    [conn start];
}
    
#endif

} // namespace NetworkUtil
} // namespace Zephyros
