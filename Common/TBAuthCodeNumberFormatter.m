//
//  TBAuthCodeNumberFormatter.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBAuthCodeNumberFormatter.h"

@implementation TBAuthCodeNumberFormatter

- (instancetype)init {
    if((self = [super init])) {
        [self setNumberStyle:NSNumberFormatterDecimalStyle];
        [self setMaximumIntegerDigits:5];
        [self setMinimumIntegerDigits:5];
        [self setMinimum:@0];
        [self setUsesGroupingSeparator:NO];
        [self setAllowsFloats:NO];
    }
    return self;
}

@end
