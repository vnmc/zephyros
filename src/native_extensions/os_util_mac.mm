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


#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <glob.h>

#import "base/ZPYAppDelegate.h"
#import "base/ZPYMenuHandler.h"

#ifdef USE_CEF
#import "base/cef/client_handler.h"
#import "base/cef/extension_handler.h"
#endif

#import "components/ZPYMenuItem.h"
#import "components/ZPYTouchBarItems.h"

#import "native_extensions/os_util.h"
#import "native_extensions/image_util_mac.h"

#import "util/string_util.h"


#define TYPE_STDOUT 0
#define TYPE_STDERR 2

#ifdef USE_CEF
extern CefRefPtr<Zephyros::ClientHandler> g_handler;
#endif

#ifdef USE_WEBVIEW
extern JSContextRef g_ctx;
#endif

ZPYMenuHandler* g_menuHandler = nil;


@interface MenuDummyView : NSView

- (void) menuItemSelected: (id) sender;
@property NSString *selectedCommandId;

@end

@implementation MenuDummyView

- (void) menuItemSelected: (id) sender
{
    _selectedCommandId = [sender commandId];
}

@end


@interface StreamData : NSObject

@property NSMutableData *data;
@property int type;

- (id) initWithData: (NSData*) data type: (int) type;
- (void) appendData: (NSData*) data;

@end


@implementation StreamData

- (id) initWithData: (NSData*) data type: (int) type
{
    self = [super init];
    
    _data = [[NSMutableData alloc] init];
    [_data appendData: data];
    _type = type;
    
    return self;
}

- (void) appendData: (NSData*) data
{
    [_data appendData: data];
}

@end


@interface ProcessManager : NSObject

@property NSTask *task;

@property NSFileHandle *fileHandleInPipe;
@property NSFileHandle *fileHandleOutPipe;
@property NSFileHandle *fileHandleErrPipe;

@property NSMutableArray *data;

@property CallbackId callback;

- (id) init: (CallbackId) callback;
- (void) start: (NSString*) executablePath arguments: (NSArray*) args currentDirectory: (NSString*) cwd;
- (void) readPipe: (NSNotification*) notification;

@end

@implementation ProcessManager

- (id) init: (CallbackId) callback
{
    self = [super init];
    
    _data = [[NSMutableArray alloc] init];
    
#ifdef USE_WEBVIEW
    JSValueProtect(g_ctx, callback);
#endif
    
    _callback = callback;
    
    [[NSNotificationCenter defaultCenter] addObserver: self
                                             selector: @selector(readPipe:)
                                                 name: NSFileHandleReadCompletionNotification object: nil];
    
    return self;
}

- (void) start: (NSString*) executablePath arguments: (NSArray*) args currentDirectory: (NSString*) cwd
{
    _task = [[NSTask alloc] init];
    
    // glob the executable path
    // (e.g., '~/.gem/ruby/*/bin/sass' will be resolved to something like '/Users/christen/.gem/ruby/2.0.0/bin/sass')
    glob_t g;
    if (glob([executablePath UTF8String], GLOB_TILDE, NULL, &g) == 0 && g.gl_matchc > 0)
        executablePath = [NSString stringWithUTF8String: g.gl_pathv[0]];
    globfree(&g);

    if (glob([cwd UTF8String], GLOB_TILDE, NULL, &g) == 0 && g.gl_matchc > 0)
        cwd = [NSString stringWithUTF8String: g.gl_pathv[0]];
    globfree(&g);

    // configure the task
    _task.launchPath = executablePath;
    _task.arguments = args;
    _task.currentDirectoryPath = cwd;
    
    // set up the pipes
    NSPipe *pipeIn = [NSPipe pipe];
    NSPipe *pipeOut = [NSPipe pipe];
    NSPipe *pipeErr = [NSPipe pipe];
    
    _task.standardInput = pipeIn;
    _task.standardOutput = pipeOut;
    _task.standardError = pipeErr;
    
    _fileHandleInPipe = [pipeIn fileHandleForWriting];
    _fileHandleOutPipe = [pipeOut fileHandleForReading];
    _fileHandleErrPipe = [pipeErr fileHandleForReading];
    
    // invoke the callback after the process has terminated
    ProcessManager *me = self;
    _task.terminationHandler = ^(NSTask *task)
    {
        // allow finish reading the output
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.2 * NSEC_PER_SEC), dispatch_get_main_queue(),
        ^{
            Zephyros::JavaScript::Array stream = Zephyros::JavaScript::CreateArray();
            int i = 0;
            for (StreamData* d in me.data)
            {
                Zephyros::JavaScript::Object obj = Zephyros::JavaScript::CreateObject();
                obj->SetInt("fd", d.type);
                obj->SetString("text", [[[NSString alloc] initWithData: d.data encoding: NSUTF8StringEncoding] UTF8String]);
                stream->SetDictionary(i++, obj);
            }

#ifdef USE_CEF
            Zephyros::JavaScript::Array args = Zephyros::JavaScript::CreateArray();
            args->SetInt(0, [me.task terminationStatus]);
            args->SetList(1, stream);
            g_handler->GetClientExtensionHandler()->InvokeCallback(me.callback, args);
#endif
                
#ifdef USE_WEBVIEW
            if (JSObjectIsFunction(g_ctx, me.callback))
            {
                JSValueRef args[] = {
                    JSValueMakeNumber(g_ctx, [me.task terminationStatus]),
                    stream->AsJS()
                };
            
                JSObjectCallAsFunction(g_ctx, me.callback, NULL, 2, args, NULL);
            }

            JSValueUnprotect(g_ctx, me.callback);
#endif

            [[NSNotificationCenter defaultCenter] removeObserver: me];
        });
    };
    
    // launch the process
    @try
    {
        [_task launch];
    }
    @catch (NSException *exception)
    {
        // the process couldn't be launched
        
#ifdef USE_CEF
        Zephyros::JavaScript::Array args = Zephyros::JavaScript::CreateArray();
        args->SetNull(0);
        g_handler->GetClientExtensionHandler()->InvokeCallback(_callback, args);
#endif
        
#ifdef USE_WEBVIEW
        JSValueRef args[] = {
            JSValueMakeNull(g_ctx),
            JSValueMakeUndefined(g_ctx),
            JSValueMakeUndefined(g_ctx)
        };
        
        JSObjectCallAsFunction(g_ctx, _callback, NULL, 3, args, NULL);
        JSValueUnprotect(g_ctx, _callback);
#endif
        
        [[NSNotificationCenter defaultCenter] removeObserver: self];
    }
    
    // start reading the process's output
    [_fileHandleOutPipe readInBackgroundAndNotify];
    [_fileHandleErrPipe readInBackgroundAndNotify];
}

- (void) recordData: (NSData*) data type: (int) type
{
    if (data.length == 0)
        return;
    
    NSUInteger len = _data.count;
    if (len > 0 && ((StreamData*) _data[len - 1]).type == type)
        [_data[len - 1] appendData: data];
    else
        [_data addObject: [[StreamData alloc] initWithData: data type: type]];
}

- (void) readPipe: (NSNotification*) notification
{
    NSData *data = [[notification userInfo] objectForKey: NSFileHandleNotificationDataItem];
    
    if ([notification object] == _fileHandleOutPipe)
    {
        [self recordData: data type: TYPE_STDOUT];
        [_fileHandleOutPipe readInBackgroundAndNotify];
    }
    else if ([notification object] == _fileHandleErrPipe)
    {
        [self recordData: data type: TYPE_STDERR];
        [_fileHandleErrPipe readInBackgroundAndNotify];
    }
}

@end


namespace Zephyros {
namespace OSUtil {
    
String GetOSVersion()
{
    StringStream ssVersion;
    ssVersion << TEXT("MacOSX-");
    
    NSString *osVersion = [[NSProcessInfo processInfo] operatingSystemVersionString];
    NSUInteger len = [osVersion length];
    
    // parse the OS version string:
    // start at the first digit and end after the sequence of digits and dots
    
    int start = -1;
    for (NSUInteger i = 0; i < len; ++i)
        if (isdigit([osVersion characterAtIndex: i]))
        {
            start = (int) i;
            break;
        }

    if (start >= 0)
    {
        int end = -1;
        for (NSUInteger i = start + 1; i < len; ++i)
        {
            unichar c = [osVersion characterAtIndex: i];
            if (!isdigit(c) && c != '.')
            {
                end = (int) i;
                break;
            }
        }
        
        if (end >= 0)
            ssVersion << [[osVersion substringWithRange: NSMakeRange(start, end - start)] UTF8String];
    }
    
    return ssVersion.str();
}

String GetUserName()
{
    return [NSUserName() UTF8String];
}
    
String GetHomeDirectory()
{
    return [NSHomeDirectory() UTF8String];
}
    
String GetComputerName()
{
    CFStringRef computerName = SCDynamicStoreCopyComputerName(NULL, NULL);
    String ret([((__bridge NSString*) computerName) UTF8String]);
    CFRelease(computerName);
    return ret;
}
 
void StartProcess(CallbackId callback, String executableFileName, std::vector<String> arguments, String cwd)
{
    ProcessManager *processManager = [[ProcessManager alloc] init: callback];
    
    NSMutableArray *args = [[NSMutableArray alloc] init];
    for (String arg : arguments)
        [args addObject: [NSString stringWithUTF8String: arg.c_str()]];
    
    [processManager start: [NSString stringWithUTF8String: executableFileName.c_str()]
                arguments: args
         currentDirectory: [NSString stringWithUTF8String: cwd.c_str()]];
}
 
    
static int lastOriginX = INT_MIN;
static int lastOriginY = INT_MIN;
static int lastX = -1;
static int lastY = -1;
static int lastWidth = -1;
static int lastHeight = -1;
    
#define min(a,b) ((a)<(b) ? (a) : (b))
#define max(a,b) ((a)<(b) ? (b) : (a))
    
void SetWindowSize(CallbackId callback, int width, int height, bool hasWidth, bool hasHeight, int* pNewWidth, int* pNewHeight)
{
#ifdef USE_WEBVIEW
    JSValueProtect(g_ctx, callback);
#endif
    
    int screenMinX = INT_MAX;
    int screenMinY = INT_MAX;
    int screenMaxX = 0;
    int screenMaxY = 0;
    
    NSScreen *mainScreen = [NSScreen mainScreen];
    int mainScreenWidth = mainScreen.visibleFrame.size.width;
    int mainScreenHeight = mainScreen.visibleFrame.size.height;
    
    // get the total size of all screens
    NSArray *screens = [NSScreen screens];
    for (unsigned int i = 0; i < screens.count; ++i)
    {
        NSRect screenFrame = [screens[i] visibleFrame];

        screenMinX = min(screenMinX, screenFrame.origin.x);
        screenMinY = min(screenMinY, screenFrame.origin.y);
        screenMaxX = max(screenMaxX, screenFrame.origin.x + screenFrame.size.width);
        screenMaxY = max(screenMaxY, screenFrame.origin.y + screenFrame.size.height);
    }
    
    // get the current window frame
    NSWindow *window = [(ZPYAppDelegate*) [[NSApplication sharedApplication] delegate] window];
    NSRect frame = window.frame;
    bool isFullScreen = ([window styleMask] & NSFullScreenWindowMask) != 0;
    bool windowMoved = frame.origin.x != lastX || frame.origin.y != lastY;
    
    // if the width is set, set the window's width
    if (hasWidth)
    {
        if (isFullScreen)
            width = mainScreenWidth;
            
        frame.size.width = width;
    
        // do we need to restore the last origin.x?
        if (lastWidth > width && lastOriginX != INT_MIN)
        {
            if (!windowMoved)
                frame.origin.x = lastOriginX;
            lastOriginX = INT_MIN;
        }
        
        // adjust the window's origin if, after resizing, the window would fall off the screen
        if ((frame.origin.x + width) > screenMaxX)
        {
            int newOriginX = screenMaxX - width;
            if (newOriginX < screenMinX)
            {
                newOriginX = screenMinX;
                frame.size.width = screenMaxX;
            }
            
            if (frame.origin.x != newOriginX)
            {
                lastOriginX = frame.origin.x;
                frame.origin.x = newOriginX;
            }
        }
        
        // save the last width
        lastWidth = width;
    }
    else
        lastWidth = -1;
    
    // if the height is set, set the window's height
    if (hasHeight)
    {
        if (isFullScreen)
            height = mainScreenHeight;
        
        frame.size.height = height;

        if (lastHeight > height && lastOriginY != INT_MIN)
        {
            if (!windowMoved)
                frame.origin.y = lastOriginY;
            lastOriginY = INT_MIN;
        }
        
        if ((frame.origin.y + height) > screenMaxY)
        {
            int newOriginY = screenMaxY - height;
            if (newOriginY < screenMinY)
            {
                newOriginY = screenMinY;
                frame.size.height = screenMaxY;
            }
            
            if (frame.origin.y != newOriginY)
            {
                lastOriginY = frame.origin.y;
                frame.origin.y = newOriginY;
            }
        }
        
        lastHeight = height;
    }
    else
        lastHeight = -1;
    
    // memorize the last window position
    lastX = frame.origin.x;
    lastY = frame.origin.y;
    
    // actually resize the window
    if (hasWidth || hasHeight)
    {
        [window setFrame: frame display: YES animate: YES];
        
        // call the callback (if any was passed), after the resizing is done
        float time = [[NSUserDefaults standardUserDefaults] floatForKey: @"NSWindowResizeTime"];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, time * NSEC_PER_SEC), dispatch_get_main_queue(),
        ^{
            JavaScript::Object newSize = JavaScript::CreateObject();
            newSize->SetInt("width", frame.size.width);
            newSize->SetInt("height", frame.size.height);

#ifdef USE_CEF
            JavaScript::Array args = JavaScript::CreateArray();
            args->SetDictionary(0, newSize);
            g_handler->GetClientExtensionHandler()->InvokeCallback(callback, args);
#endif
            
#ifdef USE_WEBVIEW
            if (JSObjectIsFunction(g_ctx, callback))
            {
                JSValueRef arg = newSize->AsJS();
                JSObjectCallAsFunction(g_ctx, callback, NULL, 1, &arg, NULL);
            }
            
            JSValueUnprotect(g_ctx, callback);
#endif
        });
    }

    *pNewWidth = window.frame.size.width;
    *pNewHeight = window.frame.size.height;
}
    
    
void SetMinimumWindowSize(int width, int height)
{
    NSWindow *window = [(ZPYAppDelegate*) [[NSApplication sharedApplication] delegate] window];
    [window setMinSize: NSMakeSize(width, height)];
}
    
    
void DisplayNotification(String title, String details)
{
    NSUserNotification *notification = [[NSUserNotification alloc] init];
        
    notification.title = [NSString stringWithUTF8String: title.c_str()];
    notification.informativeText = [NSString stringWithUTF8String: details.c_str()];
    notification.soundName = NSUserNotificationDefaultSoundName;
       
    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification: notification];
}
    
void RequestUserAttention()
{
    [NSApp requestUserAttention: NSCriticalRequest];
}

void CreateMenuRecursive(NSMenu* menuParent, JavaScript::Array menuItems, ZPYMenuHandler* menuHandler, bool bIsInDemoMode)
{
    int pos = 0;
    bool bPrevItemWasSeparator = false;
    int numItems = (int) menuItems->GetSize();
    
    for (int i = 0; i < numItems; ++i)
    {
        JavaScript::Object item = menuItems->GetDictionary(i);
        NSMenuItem* menuItem = nil;
        
        // get the caption (get rid of underscores; Mac doesn't support underlined characters)
        String caption = StringReplace(item->GetString("caption"), "_", "");
        
        if (caption == TEXT("-"))
        {
            // this menu item is a separator
            
            // don't add a separator if the previous item already was one
            if (bPrevItemWasSeparator)
                continue;
            
            // add the separator
            menuItem = [NSMenuItem separatorItem];
            bPrevItemWasSeparator = true;
        }
        else
        {
            if (item->HasKey("systemCommandId"))
            {
                // set the menu action to the selector corresponding to systemCommandId,
                // but don't set target => the target will be the first responder
                menuItem = [[ZPYMenuItem alloc] init];
                menuItem.action = NSSelectorFromString([NSString stringWithUTF8String: String(item->GetString("systemCommandId")).c_str()]);
            }
            else
            {
                NSString *commandId = [NSString stringWithUTF8String: String(item->GetString("menuCommandId")).c_str()];
                
                // don't add licensing related menu items if not in demo mode
                if (!bIsInDemoMode && ([commandId isEqualToString: @MENUCOMMAND_ENTER_LICENSE] || [commandId isEqualToString: @MENUCOMMAND_PURCHASE_LICENSE]))
                    continue;
                
                menuItem = [[ZPYMenuItem alloc] init];
                
                // special command IDs
                if ([commandId isEqualToString: @MENUCOMMAND_TERMINATE])
                {
                    menuItem.action = @selector(terminate:);
                    menuItem.target = [NSApplication sharedApplication];
                }
#ifndef APPSTORE
                else if ([commandId isEqualToString: @MENUCOMMAND_CHECK_UPDATE])
                {
                    menuItem.action = @selector(checkForUpdates:);
                    menuItem.target = ((ZPYAppDelegate*) [[NSApplication sharedApplication] delegate]).updater;
                }
#endif
                else
                {
                    ((ZPYMenuItem*) menuItem).commandId = commandId;
                    menuItem.action = @selector(menuItemSelected:);
                    menuItem.target = menuHandler;
                }
            }
            
            menuItem.title = [NSString stringWithUTF8String: caption.c_str()];
            
            if (item->HasKey("key"))
            {
                String strKey = item->GetString("key");
                String strKeyLc = ToLower(strKey);
                
                unichar ch = 0;
                if (strKeyLc == "left")
                    ch = NSLeftArrowFunctionKey;
                else if (strKeyLc == "right")
                    ch = NSRightArrowFunctionKey;
                else if (strKeyLc == "up")
                    ch = NSUpArrowFunctionKey;
                else if (strKeyLc == "down")
                    ch = NSDownArrowFunctionKey;
                else if (strKeyLc == "page up" || strKeyLc == "pageup")
                    ch = NSPageUpFunctionKey;
                else if (strKeyLc == "page down" || strKeyLc == "pagedown")
                    ch = NSPageDownFunctionKey;
                else if (strKeyLc == "home")
                    ch = NSHomeFunctionKey;
                else if (strKeyLc == "end")
                    ch = NSEndFunctionKey;
                else if (strKeyLc == "insert")
                    ch = NSInsertFunctionKey;
                else if (strKeyLc == "delete")
                    ch = NSDeleteFunctionKey;
                else if (strKeyLc.length() > 0 && strKeyLc.at(0) == 'f' && isdigit(strKeyLc.at(1)))
                    ch = NSF1FunctionKey + atoi(strKey.substr(1).c_str()) - 1;

                menuItem.keyEquivalent = ch ?
                    [NSString stringWithCharacters: &ch length: 1] :
                    [NSString stringWithUTF8String: strKey.c_str()];
            
                if (item->HasKey("keyModifiers"))
                {
                    int nModifiers = item->GetInt("keyModifiers");
                    int nMask = 0;
                
                    if (nModifiers & 1)
                        nMask |= NSShiftKeyMask;
                    if (nModifiers & 2)
                        nMask |= NSCommandKeyMask;
                    if (nModifiers & 4)
                        nMask |= NSAlternateKeyMask;
                    if (nModifiers & 8)
                        nMask |= NSControlKeyMask;
                
                    menuItem.keyEquivalentModifierMask = nMask;
                }
            }

            if (item->HasKey("image"))
            {
                String image = item->GetString("image");
                if (image.length() > 0)
                    menuItem.image = ImageUtil::Base64EncodedPNGToNSImage(image, NSMakeSize(19, 19));
            }
            
            if (item->HasKey("subMenuItems"))
            {
                NSMenu* menu = [[NSMenu alloc] init];
                menu.title = menuItem.title;
                menuItem.submenu = menu;
                CreateMenuRecursive(menu, item->GetList("subMenuItems"), menuHandler, bIsInDemoMode);
            }
        }
        
        [menuParent insertItem: menuItem atIndex: pos++];
        bPrevItemWasSeparator = false;
    }
}
    
void CreateMenu(JavaScript::Array menuItems)
{
    if (!g_menuHandler)
        g_menuHandler = [[ZPYMenuHandler alloc] init];
    
    NSMenu* mainMenu = [[NSMenu alloc] init];
    CreateMenuRecursive(mainMenu, menuItems, g_menuHandler, Zephyros::GetLicenseManager() != NULL && Zephyros::GetLicenseManager()->IsInDemoMode());
    [NSApp setMainMenu: mainMenu];
}
    
bool RemoveMenuItemRecursive(NSMenu* menu, String& strCommandId)
{
    if (!menu)
        return false;
    
    int idx = 0;
    for (NSMenuItem* item in [menu itemArray])
    {
        if ([item isKindOfClass: ZPYMenuItem.class] && strCommandId == [((ZPYMenuItem*) item).commandId UTF8String])
        {
            [menu removeItemAtIndex: idx];
            return true;
        }
        
        if (item.menu && RemoveMenuItemRecursive(item.menu, strCommandId))
            return true;
        
        ++idx;
    }
    
    return false;
}
    
void RemoveMenuItem(String strCommandId)
{
    RemoveMenuItemRecursive([NSApp mainMenu], strCommandId);
}

    
static NSMutableArray* g_arrContextMenus = [[NSMutableArray alloc] init];
 
MenuHandle CreateContextMenu(JavaScript::Array menuItems)
{
    NSMenu* menu = [[NSMenu alloc] init];
    menu.autoenablesItems = NO;
    
    MenuHandle menuHandle = (MenuHandle) g_arrContextMenus.count;
    [g_arrContextMenus addObject: menu];
    
    CreateMenuRecursive(menu, menuItems, nil, false);
    
    return menuHandle;
}
    
String ShowContextMenu(MenuHandle nMenuHandle, int x, int y)
{
    if (nMenuHandle >= g_arrContextMenus.count)
        return "";
    
    NSPoint pt = NSMakePoint(x - 1, [[[NSApp mainWindow] contentView] frame].size.height - y - 4);
        
    NSMenu* menu = g_arrContextMenus[nMenuHandle];
    NSEvent* event = [NSEvent otherEventWithType: NSApplicationDefined
                                        location: pt
                                   modifierFlags: NSApplicationDefined
                                       timestamp: (NSTimeInterval) 0
                                    windowNumber: [NSApp mainWindow].windowNumber
                                         context: [NSGraphicsContext currentContext]
                                         subtype: 0
                                           data1: 0
                                           data2: 0];
        
    MenuDummyView* view = [[MenuDummyView alloc] initWithFrame: NSMakeRect(0, 0, 1, 1)];
        
    [NSMenu popUpContextMenu: menu withEvent: event forView: view];
    return view.selectedCommandId == nil ? "" : String([view.selectedCommandId UTF8String]);
}
    
int HexCharToInt(char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
    if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
    return 0;
}
    
float ToColorComponent(const char* str)
{
    return (HexCharToInt(str[0]) * 16 + HexCharToInt(str[1])) / 255.f;
}
    
void CreateTouchBar(JavaScript::Array touchBarItems)
{
    if (![[NSApplication sharedApplication] respondsToSelector: @selector(isAutomaticCustomizeTouchBarMenuItemEnabled)])
        return;

    ZPYAppDelegate* appDelegate = (ZPYAppDelegate*) [[NSApplication sharedApplication] delegate];
    NSMutableArray* ids = [[NSMutableArray alloc] init];
    int numItems = (int) touchBarItems->GetSize();
    
    [appDelegate.touchBarHandler clear];
    
    for (int i = 0; i < numItems; ++i)
    {
        JavaScript::Object item = touchBarItems->GetDictionary(i);
        if (!item->HasKey("id") || !item->HasKey("caption"))
            continue;

        NSString* identifier = [NSString stringWithUTF8String: item->GetString("id").c_str()];
        
        // add the identifier to the list
        [ids addObject: identifier];

        // create the image
        NSImage* image = nil;
        if (item->HasKey("image"))
        {
            String imgStr = item->GetString("image");
            if (!imgStr.empty())
                image = ImageUtil::Base64EncodedPNGToNSImage(imgStr, NSMakeSize(18, 18));
        }
        
        // create the text color
        NSColor* color = nil;
        if (item->HasKey("color"))
        {
            const char* col = item->GetString("color").c_str();
            if (col[0] == '#' && strlen(col) >= 7)
            {
                color = [NSColor colorWithCalibratedRed: ToColorComponent(col + 1)
                                                  green: ToColorComponent(col + 3)
                                                   blue: ToColorComponent(col + 5)
                                                  alpha: 1.f];
            }
        }

        // create the background color
        NSColor* colorBkgnd = nil;
        if (item->HasKey("backgroundColor"))
        {
            const char* col = item->GetString("backgroundColor").c_str();
            if (col[0] == '#' && strlen(col) >= 7)
            {
                colorBkgnd = [NSColor colorWithCalibratedRed: ToColorComponent(col + 1)
                                                       green: ToColorComponent(col + 3)
                                                        blue: ToColorComponent(col + 5)
                                                       alpha: 1.f];
            }
        }
        
        // create the button
        [appDelegate.touchBarHandler addItem: [ZPYTouchBarButton buttonWithId: identifier
                                                                        title: [NSString stringWithUTF8String: item->GetString("caption").c_str()]
                                                                        image: image
                                                                        color: color
                                                              backgroundColor: colorBkgnd
                                                                       action: @selector(touchBarItemSelected:)
                                                                       target: appDelegate.touchBarHandler]];
    }

    [ids addObject: NSTouchBarItemIdentifierOtherItemsProxy];
    appDelegate.touchBar.defaultItemIdentifiers = ids;
}
    
void CopyToClipboard(String text)
{
    NSPasteboard* pasteBoard = [NSPasteboard generalPasteboard];
    [pasteBoard declareTypes: @[ NSStringPboardType ] owner: nil];
    [pasteBoard setString: [NSString stringWithUTF8String: text.c_str()] forType: NSStringPboardType];
}

void CleanUp()
{
    // not needed on Mac
}

} // namespace OSUtil
} // namespace Zephyros

