//
//  TBAuthsArrayController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBAuthsArrayController.h"
#import "NSDateFormatter+ISO8601.h"

@implementation TBAuthsArrayController

- (id)newObject {
    NSNumber* code = @12345;
    NSString* name = @"New Device";
    NSString* added_at = [[NSDateFormatter dateFormatterWithISO8601Format] stringFromDate:[NSDate date]];

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:code, @"code", name, @"name", added_at, @"added_at", nil];
}

@end
