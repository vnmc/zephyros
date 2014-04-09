//
//  GLMenuItem.m
//  Zephyros
//
//  Created by Matthias Christen on 20.09.13.
//
//

#import "GLMenuItem.h"

@implementation GLMenuItem

- (id) initWithTitle: (NSString*) title action: (SEL) selector target: (id) target commandId: (NSString*) cmd
{
    self = [super initWithTitle: title action: selector keyEquivalent: @""];
    self.target = target;
    self.commandId = cmd;
    return self;
}

@end
