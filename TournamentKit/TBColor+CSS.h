//
//  TBColor+CSS.h
//  TournamentKit
//
//  Created by Ryan Drake on 2/17/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <TargetConditionals.h>

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#define TBColor UIColor
#else
#import <Cocoa/Cocoa.h>
#define TBColor NSColor
#endif

@interface TBColor (TBColor_CSS)

// Lookup a color using css 3/svg color name, or RGB hex
+ (TBColor*)colorWithName:(NSString*)colorName;

// Get a css 3/svg color name for color
- (NSString*)name;

@end
