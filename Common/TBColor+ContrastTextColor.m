//
//  TBColor+ContrastTextColor.m
//  td
//
//  Created by Ryan Drake on 12/14/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBColor+ContrastTextColor.h"

@implementation TBColor (ContrastTextColor)

- (TBColor*)contrastTextColor {
    CGFloat r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
    [self getRed:&r green:&g blue:&b alpha:&a];
    double luminance = 0.299f * r + 0.587f * g + 0.114f * b;
    return luminance > 0.5 ? [TBColor blackColor] : [TBColor whiteColor];
}

@end
