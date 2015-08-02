//
//  TBFundingArrayController.m
//  td
//
//  Created by Ryan Drake on 8/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBFundingArrayController.h"

@implementation TBFundingArrayController

- (id)newObject {
    NSString* name = @"New Rebuy";
    NSNumber* is_addon = @NO;
    NSNumber* chips = @5000;
    NSNumber* cost = @10;
    NSNumber* commission = @0;
    NSNumber* equity = @10;

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:name, @"name", is_addon, @"is_addon", chips, @"chips", cost, @"cost", commission, @"commission", equity, @"equity", nil];
}

@end
