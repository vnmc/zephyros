/*Copyright (c) 2009 Extendmac, LLC. <support@extendmac.com>
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 */

#import "util/EMKeychainItem.h"

@interface EMKeychainItem (Private)

/*!
    @abstract Modifies the given attribute to be newValue.
    @param attributeTag The attribute's tag.
    @param newValue A pointer to the new value.
    @param newLength The length of the new value.
*/  
- (void)_modifyAttributeWithTag:(SecItemAttr)attributeTag toBeValue:(void *)newValue ofLength:(UInt32)newLength;

@end

@implementation EMKeychainItem

static BOOL _logsErrors;

+ (void)lockKeychain
{
    SecKeychainLock(NULL);
}

+ (void)unlockKeychain
{
    SecKeychainUnlock(NULL, 0, NULL, NO);
}

+ (BOOL)logsErrors
{
    @synchronized (self)
    {
        return _logsErrors;
    }
    return NO;
}

+ (void)setLogsErrors:(BOOL)logsErrors
{
    @synchronized (self)
    {
        if (_logsErrors == logsErrors)
            return;
        
        _logsErrors = logsErrors;
    }
}

#pragma mark -

- (id)_initWithCoreKeychainItem:(SecKeychainItemRef)item
                       username:(NSString *)username
                       password:(NSString *)password
{
    if ((self = [super init]))
    {
        mCoreKeychainItem = item;
        mUsername = [username copy];
        mPassword = [password copy];
        
        return self;
    }
    return nil;
}

- (void)_modifyAttributeWithTag:(SecItemAttr)attributeTag toBeValue:(void *)newValue ofLength:(UInt32)newLength
{
    NSAssert(mCoreKeychainItem, @"Core keychain item is nil. You cannot modify a keychain item that is not in the keychain.");
    
    SecKeychainAttribute attributes[1];
    attributes[0].tag = attributeTag;
    attributes[0].length = newLength;
    attributes[0].data = newValue;
    
    SecKeychainAttributeList attributeList;
    attributeList.count = 1;
    attributeList.attr = attributes;
    
    SecKeychainItemModifyAttributesAndData(mCoreKeychainItem, &attributeList, 0, NULL);
}

- (void)dealloc
{
    if (mCoreKeychainItem)
        CFRelease(mCoreKeychainItem);
}

#pragma mark -
#pragma mark General Properties
@dynamic password;
- (NSString *)password
{
    @synchronized (self)
    {
        return [mPassword copy];
    }
}

- (void)setPassword:(NSString *)newPassword
{
    @synchronized (self)
    {
        if (mPassword == newPassword)
            return;
        
        mPassword = [newPassword copy];
        
        const char *newPasswordCString = [newPassword UTF8String];
        SecKeychainItemModifyAttributesAndData(mCoreKeychainItem, NULL, (UInt32) strlen(newPasswordCString), (void*) newPasswordCString);
    }
}

#pragma mark -
@dynamic username;
- (NSString *)username
{
    @synchronized (self)
    {
        return [mUsername copy];
    }
}

- (void)setUsername:(NSString *)newUsername
{
    @synchronized (self)
    {
        if (mUsername == newUsername)
            return;
        
        mUsername = [newUsername copy];
        
        const char *newUsernameCString = [newUsername UTF8String];
        [self _modifyAttributeWithTag: kSecAccountItemAttr toBeValue: (void*) newUsernameCString ofLength: (UInt32) strlen(newUsernameCString)];
    }
}

#pragma mark -
@dynamic label;
- (NSString *)label
{
    @synchronized (self)
    {
        return [mLabel copy];
    }
}

- (void)setLabel:(NSString *)newLabel
{
    @synchronized (self)
    {
        if (mLabel == newLabel)
            return;
        
        mLabel = [newLabel copy];
        
        const char *newLabelCString = [newLabel UTF8String];
        [self _modifyAttributeWithTag: kSecLabelItemAttr toBeValue: (void*) newLabelCString ofLength: (UInt32) strlen(newLabelCString)];
    }
}

#pragma mark -
#pragma mark Actions
- (void)removeFromKeychain
{
    NSAssert(mCoreKeychainItem, @"Core keychain item is nil. You cannot remove a keychain item that is not in the keychain already.");
    
    if (mCoreKeychainItem)
    {
        OSStatus resultStatus = SecKeychainItemDelete(mCoreKeychainItem);
        if (resultStatus == noErr)
        {
            CFRelease(mCoreKeychainItem);
            mCoreKeychainItem = nil;
        }
    }
}

@end

#pragma mark -
@implementation EMGenericKeychainItem

- (id)_initWithCoreKeychainItem:(SecKeychainItemRef)item
                    serviceName:(NSString *)serviceName
                       username:(NSString *)username
                       password:(NSString *)password
{
    if ((self = [super _initWithCoreKeychainItem:item username:username password:password]))
    {
        mServiceName = [serviceName copy];
        return self;
    }
    return nil;
}

+ (id)_genericKeychainItemWithCoreKeychainItem:(SecKeychainItemRef)coreKeychainItem 
                                forServiceName:(NSString *)serviceName
                                      username:(NSString *)username
                                      password:(NSString *)password
{
    return [[EMGenericKeychainItem alloc] _initWithCoreKeychainItem:coreKeychainItem
                                                         serviceName:serviceName
                                                            username:username
                                                            password:password];
}

#pragma mark -
+ (EMGenericKeychainItem *)genericKeychainItemForService:(NSString *)serviceName 
                                            withUsername:(NSString *)username
{
    if (!serviceName || !username)
        return nil;
    
    const char *serviceNameCString = [serviceName UTF8String];
    const char *usernameCString = [username UTF8String];
    
    UInt32 passwordLength = 0;
    char *password = nil;
    
    SecKeychainItemRef item = nil;
    OSStatus returnStatus = SecKeychainFindGenericPassword(
        NULL,
        (UInt32) strlen(serviceNameCString),
        serviceNameCString,
        (UInt32) strlen(usernameCString),
        usernameCString,
        &passwordLength,
        (void**) &password,
        &item
    );
    
    if (returnStatus != noErr || !item)
    {
        if (_logsErrors)
        {
            NSError *err = [NSError errorWithDomain: NSOSStatusErrorDomain code: returnStatus userInfo: nil];
            NSLog(@"Error (%@) - %@", NSStringFromSelector(_cmd), err.description);
        }
        return nil;
    }
    
    NSString *passwordString = [[NSString alloc] initWithData:[NSData dataWithBytes:password length:passwordLength] encoding:NSUTF8StringEncoding];
    SecKeychainItemFreeContent(NULL, password);
    
    return [EMGenericKeychainItem _genericKeychainItemWithCoreKeychainItem:item forServiceName:serviceName username:username password:passwordString];
}

+ (EMGenericKeychainItem *)addGenericKeychainItemForService:(NSString *)serviceName
                                               withUsername:(NSString *)username
                                                   password:(NSString *)password
{
    if (!serviceName || !username || !password)
        return nil;
    
    const char *serviceNameCString = [serviceName UTF8String];
    const char *usernameCString = [username UTF8String];
    const char *passwordCString = [password UTF8String];
    
    SecKeychainItemRef item = nil;
    OSStatus returnStatus = SecKeychainAddGenericPassword(NULL, (UInt32) strlen(serviceNameCString), serviceNameCString, (UInt32) strlen(usernameCString), usernameCString, (UInt32) strlen(passwordCString), (void*) passwordCString, &item);
    
    if (returnStatus != noErr || !item)
    {
        if (_logsErrors)
        {
            NSError *err = [NSError errorWithDomain: NSOSStatusErrorDomain code: returnStatus userInfo: nil];
            NSLog(@"Error (%@) - %@", NSStringFromSelector(_cmd), err.description);
        }
        
        return nil;
    }
    return [EMGenericKeychainItem _genericKeychainItemWithCoreKeychainItem:item forServiceName:serviceName username:username password:password];
}

#pragma mark -
#pragma mark Generic Properties
@dynamic serviceName;
- (NSString *)serviceName
{
    @synchronized (self)
    {
        return [mServiceName copy];
    }
}

- (void)setServiceName:(NSString *)newServiceName
{
    @synchronized (self)
    {
        if (mServiceName == newServiceName)
            return;
        
        mServiceName = [newServiceName copy];
        
        const char *newServiceNameCString = [newServiceName UTF8String];
        [self _modifyAttributeWithTag: kSecServiceItemAttr toBeValue: (void*) newServiceNameCString ofLength: (UInt32) strlen(newServiceNameCString)];
    }
}

@end

#pragma mark -
@implementation EMInternetKeychainItem

- (id)_initWithCoreKeychainItem:(SecKeychainItemRef)item
                         server:(NSString *)server
                       username:(NSString *)username
                       password:(NSString *)password
                           path:(NSString *)path
                           port:(NSInteger)port
                       protocol:(SecProtocolType)protocol
{
    if ((self = [super _initWithCoreKeychainItem:item username:username password:password]))
    {
        mServer = [server copy];
        mPath = [path copy];
        mPort = port;
        mProtocol = protocol;
        
        return self;
    }
    return nil;
}

+ (id)_internetKeychainItemWithCoreKeychainItem:(SecKeychainItemRef)coreKeychainItem
                                      forServer:(NSString *)server
                                       username:(NSString *)username
                                       password:(NSString *)password
                                           path:(NSString *)path
                                           port:(NSInteger)port
                                       protocol:(SecProtocolType)protocol
{
    return [[EMInternetKeychainItem alloc] _initWithCoreKeychainItem:coreKeychainItem
                                                               server:server
                                                             username:username
                                                             password:password
                                                                 path:path
                                                                 port:port
                                                             protocol:protocol];
}

#pragma mark -
+ (EMInternetKeychainItem *)internetKeychainItemForServer:(NSString *)server
                                             withUsername:(NSString *)username
                                                     path:(NSString *)path
                                                     port:(NSInteger)port
                                                 protocol:(SecProtocolType)protocol
{
    if (!server || !username)
        return nil;
    
    const char *serverCString = [server UTF8String];
    const char *usernameCString = [username UTF8String];
    const char *pathCString = [path UTF8String];
    
    if (!path || [path length] == 0)
        pathCString = "";
    
    UInt32 passwordLength = 0;
    char *password = nil;
    
    SecKeychainItemRef item = nil;
    
    //0 is kSecAuthenticationTypeAny
    OSStatus returnStatus = SecKeychainFindInternetPassword(
        NULL,
        (UInt32) server.length,
        serverCString,
        0,
        NULL,
        (UInt32) username.length,
        usernameCString,
        (UInt32) path.length,
        pathCString,
        port,
        protocol,
        0,
        &passwordLength,
        (void**) &password,
        &item
    );
    
    if (returnStatus != noErr && protocol == kSecProtocolTypeFTP)
    {
        //Some clients (like Transmit) still save passwords with kSecProtocolTypeFTPAccount, which was deprecated.  Let's check for that.
        protocol = kSecProtocolTypeFTPAccount;      
        returnStatus = SecKeychainFindInternetPassword(
            NULL,
            (UInt32) server.length,
            serverCString,
            0,
            NULL,
            (UInt32) username.length,
            usernameCString,
            (UInt32) path.length,
            pathCString,
            port,
            protocol,
            0,
            &passwordLength,
            (void**) &password,
            &item
        );
    }
    
    if (returnStatus != noErr || !item)
    {
        if (_logsErrors)
        {
            NSError *err = [NSError errorWithDomain: NSOSStatusErrorDomain code: returnStatus userInfo: nil];
            NSLog(@"Error (%@) - %@", NSStringFromSelector(_cmd), err.description);
        }

        return nil;
    }
    
    // [MCH] get the account name -->
    UInt32 attributeTags[] = { kSecAccountItemAttr };
    UInt32 attributeFormat[] = { CSSM_DB_ATTRIBUTE_FORMAT_STRING };
    SecKeychainAttributeInfo attributeInfo;
    attributeInfo.count = 1;
    attributeInfo.tag = attributeTags;
    attributeInfo.format = attributeFormat;
    SecKeychainAttributeList *attributeList = nil;
    
    OSStatus attributeStatus = SecKeychainItemCopyAttributesAndData(item, &attributeInfo, NULL, &attributeList, 0, NULL);
    
    if (attributeStatus != noErr || !item)
    {
        if (_logsErrors)
        {
            NSError *err = [NSError errorWithDomain: NSOSStatusErrorDomain code: returnStatus userInfo: nil];
            NSLog(@"Error (%@) - %@", NSStringFromSelector(_cmd), err.description);
        }

        return nil;
    }
    
    SecKeychainAttribute accountNameAttribute = attributeList->attr[0];
    
    NSString* accountName = [[NSString alloc] initWithData:[NSData dataWithBytes:accountNameAttribute.data length:accountNameAttribute.length] encoding:NSUTF8StringEncoding];
    // <--

    NSString *passwordString = [[NSString alloc] initWithData:[NSData dataWithBytes:password length:passwordLength] encoding:NSUTF8StringEncoding];
    SecKeychainItemFreeContent(NULL, password);
    
    return [EMInternetKeychainItem _internetKeychainItemWithCoreKeychainItem:item forServer:server username:accountName password:passwordString path:path port:port protocol:protocol];
}

+ (EMInternetKeychainItem *)addInternetKeychainItemForServer:(NSString *)server
                                                withUsername:(NSString *)username
                                                    password:(NSString *)password
                                                        path:(NSString *)path
                                                        port:(NSInteger)port
                                                    protocol:(SecProtocolType)protocol
{
    if (!username || !server || !password)
        return nil;
    
    const char *serverCString = [server UTF8String];
    const char *usernameCString = [username UTF8String];
    const char *passwordCString = [password UTF8String];
    const char *pathCString = [path UTF8String];
    
    if (!path || [path length] == 0)
        pathCString = "";
    
    SecKeychainItemRef item = nil;
    OSStatus returnStatus = SecKeychainAddInternetPassword(
        NULL,
        (UInt32) strlen(serverCString),
        serverCString,
        0,
        NULL,
        (UInt32) strlen(usernameCString),
        usernameCString,
        (UInt32) strlen(pathCString),
        pathCString,
        port,
        protocol,
        kSecAuthenticationTypeDefault,
        (UInt32) strlen(passwordCString),
        (void*) passwordCString,
        &item
    );
    
    if (returnStatus != noErr || !item)
    {
        if (_logsErrors)
        {
            NSError *err = [NSError errorWithDomain: NSOSStatusErrorDomain code: returnStatus userInfo: nil];
            NSLog(@"Error (%@) - %@", NSStringFromSelector(_cmd), err.description);
        }

        return nil;
    }
    
    return [EMInternetKeychainItem _internetKeychainItemWithCoreKeychainItem:item forServer:server username:username password:password path:path port:port protocol:protocol];
}

#pragma mark -
#pragma mark Internet Properties
@dynamic server;
- (NSString *)server
{
    @synchronized (self)
    {
        return [mServer copy];
    }
}

- (void)setServer:(NSString *)newServer
{
    @synchronized (self)
    {
        if (mServer == newServer)
            return;
        
        mServer = [newServer copy];
        
        const char *newServerCString = [newServer UTF8String];
        [self _modifyAttributeWithTag: kSecServerItemAttr toBeValue: (void*) newServerCString ofLength: (UInt32) strlen(newServerCString)];
    }
}

#pragma mark -
@dynamic path;
- (NSString *)path
{
    @synchronized (self)
    {
        return [mPath copy];
    }
}

- (void)setPath:(NSString *)newPath
{
    if (mPath == newPath)
        return;
    
    mPath = [newPath copy];
    
    const char *newPathCString = [newPath UTF8String];
    [self _modifyAttributeWithTag: kSecPathItemAttr toBeValue: (void*) newPathCString ofLength: (UInt32) strlen(newPathCString)];
}

#pragma mark -
@dynamic port;
- (NSInteger) port
{
    @synchronized (self)
    {
        return mPort;
    }
}

- (void) setPort: (NSInteger) newPort
{
    @synchronized (self)
    {
        if (mPort == newPort)
            return;
        
        mPort = newPort;
        
        UInt32 newPortValue = (UInt32) newPort;
        [self _modifyAttributeWithTag: kSecPortItemAttr toBeValue: &newPortValue ofLength: sizeof(newPortValue)];
    }
}

#pragma mark -
@dynamic protocol;
- (SecProtocolType)protocol
{
    @synchronized (self)
    {
        return mProtocol;
    }
}

- (void)setProtocol:(SecProtocolType)newProtocol
{
    @synchronized (self)
    {
        if (mProtocol == newProtocol)
            return;
        
        mProtocol = newProtocol;
        
        [self _modifyAttributeWithTag: kSecProtocolItemAttr toBeValue: &newProtocol ofLength: sizeof(newProtocol)];
    }
}

@end
