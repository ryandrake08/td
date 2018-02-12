//
//  TBPayoutShapeNumberFormatter.m
//  TBPhone
//
//  Created by Ryan Drake on 2/12/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBPayoutShapeNumberFormatter.h"

@implementation TBPayoutShapeNumberFormatter

- (NSString*)stringForObjectValue:(id)anObject {
    if(![anObject isKindOfClass:[NSNumber class]]) {
        return nil;
    }

    double shape = [anObject doubleValue];
    if(shape <= 0.0) {
        return NSLocalizedString(@"Same To Everyone", @"Same amount of money goes to each player");
    } else if(shape < 0.25) {
        return NSLocalizedString(@"Relatively Flat", @"Flat payout structure, favorable to players just in the money");
    } else if(shape < 0.5) {
        return NSLocalizedString(@"Balanced", @"Balanced payout structure");
    } else if(shape < 0.75) {
        return NSLocalizedString(@"Top Heavy", @"Aggressive payout structure, favorable to players deep in the money");
    } else if(shape < 1.0) {
        return NSLocalizedString(@"Reward Deep", @"Very aggressive payout structure, highly favorable to players deep in the money");
    } else {
        return NSLocalizedString(@"Winner Takes All", @"Flat payout structure, favorable to players just in the money");
    }
}

- (BOOL)getObjectValue:(id*)obj forString:(NSString*)string errorDescription:(NSString**)error {
    NSLog(@"Unexpected call to TBPayoutShapeNumberFormatter getObjectValue:forString:errorDescription:");
    return NO;
}

@end
