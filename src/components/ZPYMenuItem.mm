//
//  GLMenuItem.m
//  Ghostlab
//
//  Created by Matthias Christen on 20.09.13.
//
//

#import "components/ZPYMenuItem.h"


@implementation ZPYMenuItem

- (id) initWithTitle: (NSString*) title action: (SEL) selector keyEquivalent: (NSString*) charCode
{
    self = [super initWithTitle: NSLocalizedString(title, nil) action: selector keyEquivalent: charCode];
    return self;
}

- (id) initWithTitle: (NSString*) title action: (SEL) selector target: (id) target commandId: (NSString*) cmd
{
    self = [super initWithTitle: NSLocalizedString(title, nil) action: selector keyEquivalent: @""];
    self.target = target;
    self.commandId = cmd;
    return self;
}

- (void) setTitle: (NSString*) title
{
    [super setTitle: NSLocalizedString(title, nil)];
}

- (void) awakeFromNib
{
    self.title = NSLocalizedString(self.title, nil);
}

@end
