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


#import "components/ZPYTouchBarItems.h"
#import "zephyros.h"


@implementation ZPYTouchBarButton

/*
- initWithTitle
{
    self = [[NSButton alloc] init];
}*/

+ (NSCustomTouchBarItem*) buttonWithId: (NSString*) cmdId
                                 title: (NSString*) title
                                 image: (NSImage*) image
                                 color: (NSColor*) color
                       backgroundColor: (NSColor*) backgroundColor
                                action: (SEL) selector
                                target: (id) target
{
	NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier: cmdId];

    ZPYTouchBarButton* button = [[ZPYTouchBarButton alloc] init];
    button.commandId = cmdId;
    
    if (color)
    {
        NSDictionary* attrs = @{
            NSForegroundColorAttributeName: color,
            NSFontAttributeName: [NSFont systemFontOfSize: 0]
        };

        NSMutableAttributedString* str = [[NSMutableAttributedString alloc] initWithString: title attributes: attrs];
        [str setAlignment: NSTextAlignmentCenter range: NSMakeRange(0, str.length)];
        button.attributedTitle = str;
    }
    else
    {
        button.title = title;
        button.font = [NSFont systemFontOfSize: 0];
    }
    
    if (backgroundColor)
        button.bezelColor = backgroundColor;

    if (image)
        button.image = image;

    button.target = target;
    button.action = selector;
    button.bezelStyle = NSRoundedBezelStyle;
    button.imagePosition = NSImageLeft;

    item.view = button;

    return item;
}

@end
