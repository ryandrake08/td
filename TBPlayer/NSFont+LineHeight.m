//
//  NSFont+LineHeight.m
//  td
//
//  Created by Ryan Drake on 7/5/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "NSFont+LineHeight.h"

@implementation NSFont (NSFont_LineHeight)

- (CGFloat)lineHeight {
    return [self boundingRectForFont].size.height;
}

@end
