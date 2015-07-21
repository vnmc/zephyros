//
//  GLMenu.m
//  GhostlabMac
//
//  Created by Matthias Christen on 18.03.14.
//  Copyright (c) 2014 Vanamco AG. All rights reserved.
//

#import "components/ZPYMenu.h"

@implementation ZPYMenu

- (void) awakeFromNib
{
    self.title = NSLocalizedString(self.title, nil);
}

- (void) setTitle: (NSString*) title
{
    [super setTitle: NSLocalizedString(title, nil)];
}

@end
