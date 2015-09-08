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


#import "components/ZPYWebView.h"
#import "native_extensions/path.h"


@implementation ZPYWebView

- (BOOL) performDragOperation: (id<NSDraggingInfo>) sender
{
    NSArray *urls = [[sender draggingPasteboard] readObjectsForClasses: @[ NSURL.class ]
                                                               options: @{ NSPasteboardURLReadingFileURLsOnlyKey: [NSNumber numberWithBool: NO] }];
    for (NSURL *urlRaw in urls)
    {
        NSURL *url = [urlRaw URLByStandardizingPath];
        
        if (url.isFileURL)
        {
            NSURL *urlDir = [url.absoluteString hasSuffix: @"/"] ? url : [url URLByDeletingLastPathComponent];

            NSError *error = nil;
            NSData *bookmarkData = [urlDir bookmarkDataWithOptions: NSURLBookmarkCreationWithSecurityScope
                                    includingResourceValuesForKeys: nil
                                                     relativeToURL: nil
                                                             error: &error];
            NSURL *urlWithBookmark = nil;
            if (bookmarkData != nil)
            {
                BOOL bookmarkDataIsStale = NO;
                error = nil;
                urlWithBookmark = [NSURL URLByResolvingBookmarkData: bookmarkData
                                                            options: NSURLBookmarkResolutionWithSecurityScope
                                                      relativeToURL: nil
                                                bookmarkDataIsStale: &bookmarkDataIsStale
                                                              error: &error];
            }

            Zephyros::GetNativeExtensions()->GetDroppedURLs().push_back(Zephyros::Path(
                String([url.path UTF8String]),
                urlWithBookmark == nil ? "" : String([urlWithBookmark.absoluteString UTF8String]),
                urlWithBookmark != nil
            ));
        }
        else
            Zephyros::GetNativeExtensions()->GetDroppedURLs().push_back(Zephyros::Path(String([url.absoluteString UTF8String]), "", false));
    }
    
    return [super performDragOperation: sender];
}

@end
