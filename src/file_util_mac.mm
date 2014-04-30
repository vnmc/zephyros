//
// Copyright (C) 2013-2014 Vanamco AG
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//

#include <Cocoa/Cocoa.h>
#include "file_util.h"


namespace FileUtil {

    
#ifdef USE_WEBVIEW

void ShowOpenDialog(JSObjectRef callback, BOOL canChooseFiles)
{
    JSValueProtect(g_ctx, callback);

    // create and configure an open panel
    NSOpenPanel* browsePanel = [NSOpenPanel openPanel];
    browsePanel.canChooseFiles = canChooseFiles;
    browsePanel.canChooseDirectories = YES;
    browsePanel.allowsMultipleSelection = NO;

    [browsePanel beginSheetModalForWindow: [NSApp mainWindow] completionHandler: ^(NSInteger result)
    {
        JSStringRef url = NULL;
        JSValueRef arg;

        if (result == NSFileHandlingPanelOKButton)
        {
            url = JSStringCreateWithCFString((__bridge CFStringRef) browsePanel.URL.absoluteString);
            arg = JSValueMakeString(g_ctx, url);
        }
        else
            arg = JSValueMakeNull(g_ctx);
        
        JSObjectCallAsFunction(g_ctx, callback, NULL, 1, &arg, NULL);
        
        if (url)
            JSStringRelease(url);
        
        JSValueUnprotect(g_ctx, callback);
    }];
}
    
void ShowOpenFileDialog(JSObjectRef callback)
{
    ShowOpenDialog(callback, YES);
}
    
void ShowOpenDirectoryDialog(JSObjectRef callback)
{
    ShowOpenDialog(callback, NO);
}

#else
    
bool ShowOpenDialog(Path& path, BOOL canChooseFiles)
{
    bool ret = false;
    
    // create and configure an open panel
    NSOpenPanel* browsePanel = [NSOpenPanel openPanel];
    browsePanel.canChooseFiles = canChooseFiles;
    browsePanel.canChooseDirectories = YES;
    browsePanel.allowsMultipleSelection = NO;
    
    // show the open panel
    [browsePanel beginSheetModalForWindow: [NSApp mainWindow] completionHandler: nil];
    if ([browsePanel runModal] == NSOKButton && browsePanel.URLs.count > 0)
    {
        GetPathFromNSURL(browsePanel.URL, path);
        ret = true;
    }
    
    [NSApp endSheet: browsePanel];
    return ret;
}
    
bool ShowOpenFileDialog(Path& path)
{
    return ShowOpenDialog(path, YES);
}
    
bool ShowOpenDirectoryDialog(Path& path)
{
    return ShowOpenDialog(path, NO);
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
        
#if !(__has_feature(objc_arc))
        [data release];
#endif
    }
        
    return ret;
}

} // namespace FileUtil