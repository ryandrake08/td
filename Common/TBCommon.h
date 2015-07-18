//
//  TBCommon.h
//  td
//
//  Created by Ryan Drake on 7/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <TargetConditionals.h>

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#define TBColor UIColor
#define TBFont UIFont
#define TBView UIView
#else
#import <Cocoa/Cocoa.h>
#define TBColor NSColor
#define TBFont NSFont
#define TBView NSView
#import "NSFont+LineHeight.h"
#import "NSView+BackgroundColor.h"
#endif
