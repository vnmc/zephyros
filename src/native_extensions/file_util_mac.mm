//
//  FileUtilMac.m
//  Ghostlab
//
//  Created by Matthias Christen on 10.09.13.
//
//

#include <Cocoa/Cocoa.h>

#include "native_extensions/file_util.h"
#include "native_extensions/image_util_mac.h"


namespace Zephyros {
namespace FileUtil {
    

void GetPathFromNSURL(NSURL* url, Path& path, BOOL useParentDirForSecurityBookmark)
{
    NSURL* urlDir = useParentDirForSecurityBookmark && ![url.absoluteString hasSuffix: @"/"] ? [url URLByDeletingLastPathComponent] : url;
    
    NSError *error = nil;
    NSData *bookmarkData = [urlDir bookmarkDataWithOptions: NSURLBookmarkCreationWithSecurityScope
                            includingResourceValuesForKeys: nil
                                             relativeToURL: nil
                                                     error: &error];
    NSURL *urlWithBookmark = nil;
    if (bookmarkData != nil)
    {
        BOOL bookmarkDataIsStale;
        urlWithBookmark = [NSURL URLByResolvingBookmarkData: bookmarkData
                                                    options: NSURLBookmarkResolutionWithSecurityScope
                                              relativeToURL: nil
                                        bookmarkDataIsStale: &bookmarkDataIsStale
                                                      error: &error];
    }
    else
        urlWithBookmark = url;
    
    path = Path([url.path UTF8String], [[urlWithBookmark absoluteString] UTF8String], bookmarkData != nil);
}
    
    
#ifdef USE_WEBVIEW

void ShowOpenDialog(JSObjectRef callback, BOOL canChooseFiles, BOOL useParentDirForSecurityBookmark)
{
    JSValueProtect(g_ctx, callback);

    // create and configure an open panel
    NSOpenPanel* browsePanel = [NSOpenPanel openPanel];
    browsePanel.canChooseFiles = canChooseFiles;
    browsePanel.canChooseDirectories = YES;
    browsePanel.allowsMultipleSelection = NO;

    [browsePanel beginSheetModalForWindow: [NSApp mainWindow] completionHandler: ^(NSInteger result)
    {
        JSValueRef arg;

        if (result == NSFileHandlingPanelOKButton)
        {
            Path path;
            GetPathFromNSURL(browsePanel.URL, path, useParentDirForSecurityBookmark);
            arg = path.CreateJSRepresentation()->AsJS();
        }
        else
            arg = JSValueMakeNull(g_ctx);
        
        JSObjectCallAsFunction(g_ctx, callback, NULL, 1, &arg, NULL);
        JSValueUnprotect(g_ctx, callback);
    }];
}
    
void ShowSaveFileDialog(JSObjectRef callback)
{
    // create and configure a save panel
    NSSavePanel* savePanel = [NSSavePanel savePanel];
    savePanel.canCreateDirectories = YES;
        
    // show the save panel
    [savePanel beginSheetModalForWindow: [NSApp mainWindow] completionHandler: ^(NSInteger result)
    {
        JSValueRef arg;
             
        if (result == NSFileHandlingPanelOKButton)
        {
            Path path;
            GetPathFromNSURL(savePanel.URL, path, NO);
            arg = path.CreateJSRepresentation()->AsJS();
        }
        else
            arg = JSValueMakeNull(g_ctx);
             
        JSObjectCallAsFunction(g_ctx, callback, NULL, 1, &arg, NULL);
        JSValueUnprotect(g_ctx, callback);
    }];
}

void ShowOpenFileDialog(JSObjectRef callback)
{
    ShowOpenDialog(callback, YES, NO);
}
    
void ShowOpenDirectoryDialog(JSObjectRef callback)
{
    ShowOpenDialog(callback, NO, NO);
}
    
void ShowOpenFileOrDirectoryDialog(JSObjectRef callback)
{
    ShowOpenDialog(callback, YES, YES);
}

#else
    
bool ShowOpenDialog(Path& path, BOOL canChooseFiles, BOOL useParentDirForSecurityBookmark)
{
    bool ret = false;
    
    // create and configure an open panel
    NSOpenPanel* browsePanel = [NSOpenPanel openPanel];
    browsePanel.canChooseFiles = canChooseFiles;
    browsePanel.canChooseDirectories = YES;
    browsePanel.allowsMultipleSelection = NO;
    
    // show the open panel
    [browsePanel beginSheetModalForWindow: [NSApp mainWindow] completionHandler: nil];
    if ([browsePanel runModal] == NSModalResponseOK && browsePanel.URLs.count > 0)
    {
        GetPathFromNSURL(browsePanel.URL, path, useParentDirForSecurityBookmark);
        ret = true;
    }
    
    [NSApp endSheet: browsePanel];
    return ret;
}
    
bool ShowSaveFileDialog(Path& path)
{
    bool ret = false;
    
    // create and configure a save panel
    NSSavePanel* savePanel = [NSSavePanel savePanel];
    savePanel.canCreateDirectories = YES;
        
    // show the save panel
    [savePanel beginSheetModalForWindow: [NSApp mainWindow] completionHandler: nil];
    if ([savePanel runModal] == NSOKButton)
    {
        GetPathFromNSURL(savePanel.URL, path, NO);
        ret = true;
    }
        
    [NSApp endSheet: savePanel];
    return ret;
}
    
bool ShowOpenFileDialog(Path& path)
{
    return ShowOpenDialog(path, YES, NO);
}
    
bool ShowOpenDirectoryDialog(Path& path)
{
    return ShowOpenDialog(path, NO, NO);
}
    
bool ShowOpenFileOrDirectoryDialog(Path& path)
{
    return ShowOpenDialog(path, YES, YES);
}

#endif

void ShowInFileManager(String path)
{
    // get the actual directory (the directory itself or the directory containing the file)
    NSString *dir = [NSString stringWithUTF8String: path.c_str()];
    
    NSURL *url = [NSURL URLWithString: dir];
    if (url.isFileURL)
        dir = url.path;
    else
        url = [NSURL fileURLWithPath: dir];
    
    BOOL isDir;
    if ([[NSFileManager defaultManager] fileExistsAtPath: dir isDirectory: &isDir])
    {
        if (!isDir)
            url = [url URLByDeletingLastPathComponent];
    }
    else
    {
        NSAlert *alert = [NSAlert alertWithMessageText: NSLocalizedString(@"Open in Finder", @"Open in Finder")
                                         defaultButton: NSLocalizedString(@"OK", @"OK")
                                       alternateButton: nil
                                           otherButton: nil
                             informativeTextWithFormat: NSLocalizedString(@"The path %@ does not exist.", "PathDoesNotExist"), url.path];
        
        [alert beginSheetModalForWindow: [NSApp mainWindow] modalDelegate: nil didEndSelector: nil contextInfo: nil];
    }
    
    // open in Finder
    if (url != nil)
        [[NSWorkspace sharedWorkspace] openURL: url];
}
    
bool ExistsFile(String filename)
{
    return [[NSFileManager defaultManager] fileExistsAtPath: [NSString stringWithUTF8String: filename.c_str()]] == YES;
}
    
bool IsDirectory(String path)
{
    NSString *filename = [NSString stringWithUTF8String: path.c_str()];
    
    BOOL isDirectory = NO;
    if (![[NSFileManager defaultManager] fileExistsAtPath: filename isDirectory: &isDirectory])
        return false;
    
    return isDirectory == YES;
}

bool MakeDirectory(String path, bool recursive)
{
    if (![[NSFileManager defaultManager] createDirectoryAtPath: [NSString stringWithUTF8String: path.c_str()] withIntermediateDirectories:recursive attributes:nil error:nil] )
        return false;
    return true;
}

bool ReadFile(String filename, JavaScript::Object options, String& result)
{
    NSData *data = [[NSData alloc] initWithContentsOfFile: [NSString stringWithUTF8String: filename.c_str()]];
    bool ret = false;
    if (data != nil)
    {
        std::string encoding = "";
        if (options->HasKey("encoding"))
            encoding = options->GetString("encoding");
        
        if (encoding == "" || encoding == "utf-8" || encoding == "text/plain;utf-8")
        {
            // plain text
            NSString *text = [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding];
            if (text == nil)
                text = [[NSString alloc] initWithData: data encoding: NSASCIIStringEncoding];

            if (text != nil)
            {
                result = [text UTF8String];
                
#if !(__has_feature(objc_arc))
                [text release];
#endif
                
                ret = true;
            }
        }
        else if (encoding == "image/png;base64")
        {
            // base64-encoded PNG image
            NSImage *image = [[NSImage alloc] initWithData: data];
            if (image)
            {
                result = ImageUtil::NSImageToBase64EncodedPNG(image);

#if !(__has_feature(objc_arc))
                [image release];
#endif
                
                ret = true;
            }
        }
        
#if !(__has_feature(objc_arc))
        [data release];
#endif
    }

    return ret;
}

bool WriteFile(String filename, String contents)
{
    return [[NSString stringWithUTF8String: contents.c_str()] writeToFile: [NSString stringWithUTF8String: filename.c_str()]
                                                               atomically: NO
                                                                 encoding: NSUTF8StringEncoding
                                                                    error: NULL] == YES;
}
    
bool DeleteFiles(String filenames)
{
    NSFileManager *mgr = [NSFileManager defaultManager];
        
    if (filenames.find_first_of(TEXT("*?")) == String::npos)
        return [mgr removeItemAtPath: [NSString stringWithUTF8String: filenames.c_str()] error: nil] == YES;
        
    NSString *strFilenames = [NSString stringWithUTF8String: filenames.c_str()];
    NSString *path = [strFilenames stringByDeletingLastPathComponent];
        
    // list all the files in the directory
    NSError *err;
    NSArray *files = [mgr contentsOfDirectoryAtPath: path error: &err];
        
    if (files && !err)
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat: @"SELF LIKE %@", [strFilenames lastPathComponent]];
        for (NSString *filename in files)
        {
            if ([predicate evaluateWithObject: filename])
            {
                // the filename matches the predicate; try to delete the file
                if ([mgr removeItemAtPath: [path stringByAppendingPathComponent: filename] error: nil] == NO)
                    return false;
            }
        }
            
        return true;
    }
        
    return false;
}
    
bool GetDirectory(String& path)
{
    NSString *filename = [NSString stringWithUTF8String: path.c_str()];
    
    BOOL isDirectory = NO;
    if (![[NSFileManager defaultManager] fileExistsAtPath: filename isDirectory: &isDirectory])
        return false;
    
    if (!isDirectory)
    {
        filename = [filename stringByDeletingLastPathComponent];
        path = String([filename UTF8String]);
    }
    
    return true;
}

void LoadPreferences(String key, String& data)
{
    NSUserDefaults *settings = [NSUserDefaults standardUserDefaults];
    NSData *dataSettings = [settings objectForKey: [NSString stringWithUTF8String: key.c_str()]];
    NSString *res = @"";
    if (dataSettings != nil)
        res = [NSKeyedUnarchiver unarchiveObjectWithData: dataSettings];

    data = [res UTF8String];
}

void StorePreferences(String key, String data)
{
    NSUserDefaults *settings = [NSUserDefaults standardUserDefaults];
    [settings setObject: [NSKeyedArchiver archivedDataWithRootObject: [NSString stringWithUTF8String: data.c_str()]]
                 forKey: [NSString stringWithUTF8String: key.c_str()]];
    [settings synchronize];
}

bool StartAccessingPath(Path& path)
{
    if (path.HasSecurityAccessData())
        return [[NSURL URLWithString: [NSString stringWithUTF8String: path.GetURLWithSecurityAccessData().c_str()]] startAccessingSecurityScopedResource];
    return true;
}

void StopAccessingPath(Path& path)
{
    if (path.HasSecurityAccessData())
        [[NSURL URLWithString: [NSString stringWithUTF8String: path.GetURLWithSecurityAccessData().c_str()]] stopAccessingSecurityScopedResource];
}
    
void GetApplicationResourcesPath(Path& path)
{
    path = Path(String([[[NSBundle mainBundle] resourcePath] UTF8String]), TEXT(""), false);
}

} // namespace FileUtil
} // namespace Zephyros
