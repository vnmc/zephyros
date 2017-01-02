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


#import <Cocoa/Cocoa.h>

#import "base/ZPYAppDelegate.h"
#import "base/ZPYTouchBarHandler.h"
#import "base/types.h"

#import "zephyros.h"


@implementation ZPYTouchBarHandler

- (id) init
{
    self = [super init];
    m_items = [[NSMutableDictionary alloc] init];

    return self;
}

- (void) clear
{
    [m_items removeAllObjects];
}

- (void) addItem: (NSTouchBarItem*) item
{
    [m_items setObject: item forKey: item.identifier];
}

- (NSTouchBarItem*) itemForId: (NSString*) identifier
{
    return [m_items objectForKey: identifier];
}

- (int) getTagForCommandId: (String) commandId
{
    std::map<String, int>::iterator it = m_mapIDsToTags.find(commandId);

    if (it == m_mapIDsToTags.end())
    {
        int tag = m_mapIDsToTags.size() + 1;

        m_mapIDsToTags[commandId] = tag;
        m_mapTagsToIDs[tag] = commandId;
        
        return tag;
    }
    
    return it->second;
}

- (String) getCommandIdForTag: (int) tag
{
    std::map<int, String>::iterator it = m_mapTagsToIDs.find(tag);
    return it == m_mapTagsToIDs.end() ? "" : it->second;
}

/**
 * Invoked when a menu has been clicked.
 */
- (IBAction) touchBarItemSelected: (id) sender
{
    NSInteger tag = ((NSControl*) sender).tag;
    String commandId = [self getCommandIdForTag: tag];
    
    if (commandId.length() == 0)
        return;
    
    Zephyros::JavaScript::Array args = Zephyros::JavaScript::CreateArray();
    args->SetString(0, commandId);
    Zephyros::GetNativeExtensions()->GetClientExtensionHandler()->InvokeCallbacks(TEXT("onTouchBarAction"), args);
}


@end
