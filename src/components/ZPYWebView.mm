//
//  GLWebView.m
//  GhostlabMac
//
//  Created by Matthias Christen on 14.12.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#import "base/zephyros_impl.h"
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
