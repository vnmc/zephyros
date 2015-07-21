//
//  NetworkUtil.m
//  Ghostlab
//
//  Created by Matthias Christen on 06.09.13.
//
//

#import <SystemConfiguration/SystemConfiguration.h>
#import <arpa/inet.h>
#import <ifaddrs.h>
#import <net/if.h>

#import "native_extensions/network_util.h"

#import "util/GetPrimaryMACAddress.h"
#import "util/NSData+Base64.h"
#import "util/EMKeychainItem.h"


#ifdef USE_WEBVIEW
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
    bool ret = false;
    NSURL *pUrl = [NSURL URLWithString: [NSString stringWithUTF8String: url.c_str()]];
        
    CFDictionaryRef proxySettings = SCDynamicStoreCopyProxies(NULL);
    if (proxySettings != NULL)
    {
        CFArrayRef proxies = CFNetworkCopyProxiesForURL((__bridge CFURLRef) pUrl, proxySettings);
        if (proxies != NULL && CFArrayGetCount(proxies) > 0)
        {
            CFDictionaryRef bestProxy = (CFDictionaryRef) CFArrayGetValueAtIndex(proxies, 0);
            if (bestProxy != NULL)
            {
                CFStringRef proxyType = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyTypeKey);
                
                // get the proxy from the autoconfig
                if (proxyType != NULL && !CFEqual(proxyType, kCFProxyTypeNone) && CFEqual(proxyType, kCFProxyTypeAutoConfigurationURL))
                {
                    CFURLRef scriptURL = (CFURLRef) CFDictionaryGetValue(bestProxy, kCFProxyAutoConfigurationURLKey);
                    if (scriptURL != NULL)
                    {
                        CFTypeRef res;
                        CFStreamClientContext context = { 0, &res, NULL, NULL, NULL };
                            
                        // Work around <rdar://problem/5530166>.  This dummy call to
                        // CFNetworkCopyProxiesForURL initialise some state within CFNetwork
                        // that is required by CFNetworkCopyProxiesForAutoConfigurationScript.
                        CFNetworkCopyProxiesForURL((__bridge CFURLRef) pUrl, NULL);
                            
                        CFRunLoopSourceRef rls = CFNetworkExecuteProxyAutoConfigurationURL(scriptURL, (__bridge CFURLRef) pUrl, ResultCallback, &context);
                        if (rls != NULL)
                        {
                            CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
                            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0e10, false);
                            CFRunLoopRemoveSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
                            
                            CFRelease(rls);
                                
                            if (res && CFGetTypeID(res) == CFArrayGetTypeID())
                            {
                                bestProxy = (CFDictionaryRef) CFArrayGetValueAtIndex((CFArrayRef) res, 0);
                                if (bestProxy != NULL)
                                    proxyType = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyTypeKey);
                            }
                        }
                        
                        if (res != NULL)
                            CFRelease(res);
                    }
                }
                
                // parse the proxy information
                if (bestProxy != NULL && proxyType != NULL)
                {
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

                    host = "";
                    port = -1;
                    username = "";
                    password = "";

                    char buf[200];

                    CFStringRef hostStr = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyHostNameKey);
                    if (hostStr != NULL)
                    {
                        CFStringGetCString(hostStr, buf, sizeof(buf), kCFStringEncodingUTF8);
                        host = String(buf);
                    }
                    
                    CFNumberRef portNum = (CFNumberRef) CFDictionaryGetValue(bestProxy, kCFProxyPortNumberKey);
                    if (portNum != NULL)
                        CFNumberGetValue(portNum, kCFNumberIntType, &port);
                    
                    CFStringRef userStr = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyUsernameKey);
                    if (userStr != NULL)
                    {
                        CFStringGetCString(userStr, buf, sizeof(buf), kCFStringEncodingUTF8);
                        username = String(buf);
                    }

                    CFStringRef passwordStr = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyPasswordKey);
                    if (passwordStr != NULL)
                    {
                        CFStringGetCString(passwordStr, buf, sizeof(buf), kCFStringEncodingUTF8);
                        password = String(buf);
                    }
                    
                    // if no user name / password is in the dictionary, check the keychain
                    if (userStr == NULL || passwordStr == NULL)
                    {
                        EMInternetKeychainItem *keychainItem = [EMInternetKeychainItem internetKeychainItemForServer: [NSString stringWithUTF8String: host.c_str()]
                                                                                                        withUsername: @""
                                                                                                                path: @""
                                                                                                                port: port
                                                                                                            protocol: kSecProtocolTypeAny];
                        if (keychainItem != nil)
                        {
                            username = String([keychainItem.username UTF8String]);
                            password = String([keychainItem.password UTF8String]);
                        }
                    }
                }
            }
            
            CFRelease(proxies);
        }
        else if (proxies != NULL && CFArrayGetCount(proxies) == 0)
            ret = true;
        
        CFRelease(proxySettings);
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
