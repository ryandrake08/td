//
//  TBGraphics.m
//  td
//
//  Created by Ryan Drake on 1/15/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBGraphics.h"
#import <TargetConditionals.h>

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

CGContextRef TBGraphicsGetCurrentContext(void)
{
#if TARGET_OS_IPHONE
    return UIGraphicsGetCurrentContext();
#else
    return [[NSGraphicsContext currentContext] graphicsPort];
#endif
}
