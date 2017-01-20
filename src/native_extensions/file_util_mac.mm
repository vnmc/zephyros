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
 * Florian Müller, Vanamco AG
 *******************************************************************************/


#import <Cocoa/Cocoa.h>

#import "zephyros_strings.h"

#import "native_extensions/file_util.h"
#import "native_extensions/image_util_mac.h"

#import "util/NSData+Base64.h"


#ifdef USE_CEF
extern CefRefPtr<Zephyros::ClientHandler> g_handler;
#endif

#ifdef USE_WEBVIEW
extern JSContextRef g_ctx;
#endif


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


void ShowOpenDialog(CallbackId callback, BOOL canChooseFiles, BOOL useParentDirForSecurityBookmark)
{
#ifdef USE_WEBVIEW
    JSValueProtect(g_ctx, callback);
#endif

    // create and configure an open panel
    NSOpenPanel* browsePanel = [NSOpenPanel openPanel];
    browsePanel.canChooseFiles = canChooseFiles;
    browsePanel.canChooseDirectories = YES;
    browsePanel.allowsMultipleSelection = NO;

    [browsePanel beginSheetModalForWindow: [NSApp mainWindow] completionHandler: ^(NSInteger result)
    {
#ifdef USE_CEF
        JavaScript::Array args = JavaScript::CreateArray();
#endif
#ifdef USE_WEBVIEW
        JSValueRef arg;
#endif

        if (result == NSFileHandlingPanelOKButton)
        {
            Path path;
            GetPathFromNSURL(browsePanel.URL, path, useParentDirForSecurityBookmark);
            
#ifdef USE_CEF
            args->SetDictionary(0, path.CreateJSRepresentation());
#endif
#ifdef USE_WEBVIEW
            arg = path.CreateJSRepresentation()->AsJS();
#endif
        }
        else
        {
#ifdef USE_CEF
            args->SetNull(0);
#endif
#ifdef USE_WEBVIEW
            arg = JSValueMakeNull(g_ctx);
#endif
        }
        
#ifdef USE_CEF
        g_handler->GetClientExtensionHandler()->InvokeCallback(callback, args);
#endif
#ifdef USE_WEBVIEW
        JSObjectCallAsFunction(g_ctx, callback, NULL, 1, &arg, NULL);
        JSValueUnprotect(g_ctx, callback);
#endif
    }];
}

void ShowSaveFileDialog(CallbackId callback)
{
    // create and configure a save panel
    NSSavePanel* savePanel = [NSSavePanel savePanel];
    savePanel.canCreateDirectories = YES;
        
    // show the save panel
    [savePanel beginSheetModalForWindow: [NSApp mainWindow] completionHandler: ^(NSInteger result)
    {
#ifdef USE_CEF
        JavaScript::Array args = JavaScript::CreateArray();
#endif
#ifdef USE_WEBVIEW
        JSValueRef arg;
#endif
        
        if (result == NSFileHandlingPanelOKButton)
        {
            Path path;
            GetPathFromNSURL(savePanel.URL, path, NO);
            
#ifdef USE_CEF
            args->SetDictionary(0, path.CreateJSRepresentation());
#endif
#ifdef USE_WEBVIEW
            arg = path.CreateJSRepresentation()->AsJS();
#endif
        }
        else
        {
#ifdef USE_CEF
            args->SetNull(0);
#endif
#ifdef USE_WEBVIEW
            arg = JSValueMakeNull(g_ctx);
#endif
        }
      
#ifdef USE_CEF
        g_handler->GetClientExtensionHandler()->InvokeCallback(callback, args);
#endif
#ifdef USE_WEBVIEW
        JSObjectCallAsFunction(g_ctx, callback, NULL, 1, &arg, NULL);
        JSValueUnprotect(g_ctx, callback);
#endif
    }];
}

void ShowOpenFileDialog(CallbackId callback)
{
    ShowOpenDialog(callback, YES, NO);
}

void ShowOpenDirectoryDialog(CallbackId callback)
{
    ShowOpenDialog(callback, NO, NO);
}

void ShowOpenFileOrDirectoryDialog(CallbackId callback)
{
    ShowOpenDialog(callback, YES, YES);
}

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
        NSAlert *alert = [[NSAlert alloc] init];
        alert.messageText = [NSString stringWithUTF8String: Zephyros::GetString(ZS_OPEN_FINDER).c_str()];
        alert.informativeText = [NSString stringWithFormat: [NSString stringWithUTF8String: Zephyros::GetString(ZS_OPEN_PATH_DOESNT_EXIST).c_str()], [url.path UTF8String]];
        
        [alert beginSheetModalForWindow: [NSApp mainWindow] completionHandler: nil];
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
    
bool Stat(String path, StatInfo* stat)
{
    stat->isFile = false;
    stat->isDirectory = false;
    stat->fileSize = 0;
    stat->creationDate = 0;
    stat->modificationDate = 0;
    
    NSError *err = nil;
    NSDictionary *attrs = [[NSFileManager defaultManager] attributesOfItemAtPath: [NSString stringWithUTF8String: path.c_str()]
                                                                           error: &err];
    
    if (err)
        return false;
    
    NSString *fileType = [attrs objectForKey: NSFileType];
    stat->isFile = [fileType isEqualToString: NSFileTypeRegular] || [fileType isEqualToString: NSFileTypeSymbolicLink];
    stat->isDirectory = [fileType isEqualToString: NSFileTypeDirectory];
    stat->fileSize = [[attrs objectForKey: NSFileSize] longValue];
    stat->creationDate = [[attrs objectForKey: NSFileCreationDate] timeIntervalSince1970] * 1000;
    stat->modificationDate = [[attrs objectForKey: NSFileModificationDate] timeIntervalSince1970] * 1000;
    
    return true;
}

bool MakeDirectory(String path, bool recursive, Error& err)
{
    NSError* error = nil;
    BOOL ret = [[NSFileManager defaultManager] createDirectoryAtPath: [NSString stringWithUTF8String: path.c_str()]
                                         withIntermediateDirectories: recursive
                                                          attributes: nil
                                                               error: &error];
    if (error)
        err.FromError(error);

    return ret == YES;
}

bool ReadDirectory(String path, std::vector<String>& files, Error& err)
{
    bool hasWildcard = path.find_first_of(TEXT("*?")) != String::npos;

    NSFileManager *mgr = [NSFileManager defaultManager];

    NSString *strPath = [NSString stringWithUTF8String: path.c_str()];
    NSString *strBasePath = hasWildcard ? [strPath stringByDeletingLastPathComponent] : strPath;
    
    // list all the files in the directory
    NSError *error = nil;
    NSArray *arrFiles = strBasePath.length == 0 ? nil : [mgr contentsOfDirectoryAtPath: strBasePath error: &error];
    
    if (!arrFiles || error)
    {
        err.FromError(error);
        return false;
    }

    NSPredicate *predicate = hasWildcard ? [NSPredicate predicateWithFormat: @"SELF LIKE %@", [strPath lastPathComponent]] : nil;

    for (NSString *filename in arrFiles)
        if (!predicate || [predicate evaluateWithObject: filename])
            files.push_back(String([[strBasePath stringByAppendingPathComponent: filename] UTF8String]));

    return true;
}
    
bool ReadFileBinary(String filename, uint8_t** ppData, int& size, Error& err)
{
    NSError *error = nil;
    NSData *data = [NSData dataWithContentsOfFile: [NSString stringWithUTF8String: filename.c_str()]
                                          options: 0
                                            error: &error];
    
    if (data == nil || error != nil)
    {
        err.FromError(error);
        return false;
    }
    
    size = data.length;
    *ppData = new uint8_t[size];
    [data getBytes: *ppData length: size];
    
    return true;
}

bool ReadFile(String filename, JavaScript::Object options, String& result, Error& err)
{
    NSError *error = nil;
    NSData *data = [NSData dataWithContentsOfFile: [NSString stringWithUTF8String: filename.c_str()]
                                          options: 0
                                            error: &error];
    
    if (data == nil || error != nil)
    {
        err.FromError(error);
        return false;
    }

    String encoding = "";
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
            return true;
        }
        
        // couldn't decode text
        err.SetError(ERR_DECODING_FAILED, "Failed to decode data");
        return false;
    }

    if (encoding == "image/png;base64")
    {
        // base64-encoded PNG image
        NSImage *image = [[NSImage alloc] initWithData: data];
        if (image)
        {
            result = ImageUtil::NSImageToBase64EncodedPNG(image);
            return true;
        }
        
        // couldn't decode image
        err.SetError(ERR_DECODING_FAILED, "Failed to decode data");
        return false;
    }
    
    // unsupported encoding
    err.SetError(ERR_UNKNOWN_ENCODING, "The encoding \"" + encoding + "\" is not supported.");
    return false;
}

bool WriteFile(String filename, String contents, JavaScript::Object options, Error& err)
{
    String encoding = "";
    if (options->HasKey("encoding"))
        encoding = options->GetString("encoding");

    NSData *data = nil;
    
    if (encoding == "base64")
        data = [NSData dataFromBase64String: [NSString stringWithUTF8String: contents.c_str()]];
    else
        data = [[NSString stringWithUTF8String: contents.c_str()] dataUsingEncoding: NSUTF8StringEncoding];

    if (data == nil)
    {
        err.SetError(ERR_DECODING_FAILED, "Failed to decode data");
        return false;
    }

    NSString *path = [NSString stringWithUTF8String: filename.c_str()];
    
    // create the file if it doesn't exist
    if ([[NSFileManager defaultManager] createFileAtPath: path contents: nil attributes: nil] == NO)
    {
        err.FromErrno();
        return false;
    }
    
    // open the file for writing
    NSError *error = nil;
    NSFileHandle *file = [NSFileHandle fileHandleForWritingToURL: [NSURL fileURLWithPath: path]
                                                           error: &error];
    if (error != nil)
    {
        err.FromError(error);
        return false;
    }

    // write the data to the file
    bool ret = true;
    @try
    {
        [file writeData: data];
    }
    @catch (NSException *e)
    {
        err.SetError(ERR_UNKNOWN, e.reason ? [e.reason UTF8String] : "");
        ret = false;
    }

    [file closeFile];

    return ret;
}
    
bool MoveFile(String oldFilename, String newFilename, Error& err)
{
    NSError* error = nil;
    BOOL ret = [[NSFileManager defaultManager] moveItemAtPath: [NSString stringWithUTF8String: oldFilename.c_str()]
                                                       toPath: [NSString stringWithUTF8String: newFilename.c_str()]
                                                        error: &error];
    
    if (error != nil)
        err.FromError(error);
    
    return ret == YES;
}

bool DeleteFiles(String filenames, Error& err)
{
    NSFileManager *mgr = [NSFileManager defaultManager];
    NSError *error = nil;
    
    if (filenames.find_first_of(TEXT("*?")) == String::npos)
    {
        BOOL ret = [mgr removeItemAtPath: [NSString stringWithUTF8String: filenames.c_str()] error: &error];
        if (error != nil)
            err.FromError(error);
        return ret == YES;
    }
        
    NSString *strFilenames = [NSString stringWithUTF8String: filenames.c_str()];
    NSString *path = [strFilenames stringByDeletingLastPathComponent];
        
    // list all the files in the directory
    error = nil;
    NSArray *files = [mgr contentsOfDirectoryAtPath: path error: &error];
    
    if (files == nil || error != nil)
    {
        err.FromError(error);
        return false;
    }
        
    NSPredicate *predicate = [NSPredicate predicateWithFormat: @"SELF LIKE %@", [strFilenames lastPathComponent]];
    for (NSString *filename in files)
    {
        if ([predicate evaluateWithObject: filename])
        {
            // the filename matches the predicate; try to delete the file
            if ([mgr removeItemAtPath: [path stringByAppendingPathComponent: filename] error: &error] == NO)
            {
                err.FromError(error);
                return false;
            }
        }
    }
            
    return true;
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

bool StartAccessingPath(Path& path, Error& err)
{
    if (path.HasSecurityAccessData())
    {
        BOOL ret = [[NSURL URLWithString: [NSString stringWithUTF8String: path.GetURLWithSecurityAccessData().c_str()]] startAccessingSecurityScopedResource];
        err.SetError(ERR_UNKNOWN);
        return ret == YES;
    }

    return true;
}

void StopAccessingPath(Path& path)
{
    if (path.HasSecurityAccessData())
        [[NSURL URLWithString: [NSString stringWithUTF8String: path.GetURLWithSecurityAccessData().c_str()]] stopAccessingSecurityScopedResource];
}
 
void GetTempDir(Path& path)
{
    NSString *tempDir = NSTemporaryDirectory();
    if (tempDir == nil)
        tempDir = @"/tmp";

    path = Path(String([tempDir UTF8String]), TEXT(""), false);
}

String GetApplicationPath()
{
    return "";
}

void GetApplicationResourcesPath(Path& path)
{
    path = Path(String([[[NSBundle mainBundle] resourcePath] UTF8String]), TEXT(""), false);
}

} // namespace FileUtil
} // namespace Zephyros
