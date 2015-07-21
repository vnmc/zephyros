//
//  ZPYWindowDelegate.h
//  Zephyros
//
//  Created by Matthias Christen on 20.07.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#ifndef Zephyros_ZPYWindowDelegate_h
#define Zephyros_ZPYWindowDelegate_h

/**
 * Receives notifications from controls and the browser window.
 * Will delete itself when done.
 */
@interface ZPYWindowDelegate : NSObject <NSWindowDelegate>
{
    @private
    NSWindow* m_window;
}

- (void) alert: (NSString*) title withMessage: (NSString*) message;

@end

#endif
