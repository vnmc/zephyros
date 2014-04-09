//
//  main.m
//  Ghostlab
//
//  Created by Matthias Christen on 24.11.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#import <Cocoa/Cocoa.h>


#if APPSTORE != 0
#import "appstore_receipt_validation1.h"
#endif


int main(int argc, const char * argv[])
{
#if APPSTORE == 0
    return NSApplicationMain(argc, argv);
#else
    return CheckReceiptAndRun(argc, argv);
#endif
}
