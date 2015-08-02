//
//  TBChipsArrayController.m
//  td
//
//  Created by Ryan Drake on 8/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBChipsArrayController.h"
#import "TBColor+CSS.h"

@implementation TBChipsArrayController

- (id)newObject {
    NSString* color = [TBColor randomColorName];
    NSNumber* denomination = @100;
    NSNumber* count_available = @100;

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:color, @"color", denomination, @"denomination", count_available, @"count_available", nil];
}

@end
