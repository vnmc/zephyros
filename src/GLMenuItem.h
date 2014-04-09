//
//  GLMenuItem.h
//  Ghostlab
//
//  Created by Matthias Christen on 20.09.13.
//
//

#import <Cocoa/Cocoa.h>

@interface GLMenuItem : NSMenuItem

@property (copy) NSString* commandId;


- (id) initWithTitle: (NSString*) title action: (SEL) selector target: (id) target commandId: (NSString*) cmd;

@end
