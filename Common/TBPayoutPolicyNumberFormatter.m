//
//  TBPayoutPolicyNumberFormatter.m
//  td
//
//  Created by Ryan Drake on 2/4/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBPayoutPolicyNumberFormatter.h"
#import "TournamentSession.h"

@implementation TBPayoutPolicyNumberFormatter

- (NSString*)stringForObjectValue:(id)anObject {
    if(![anObject isKindOfClass:[NSNumber class]]) {
        return nil;
    } else if([anObject isEqualToNumber:kPayoutAutomatic]) {
        return NSLocalizedString(@"Automatic", nil);
    } else if([anObject isEqualToNumber:kPayoutForced]) {
        return NSLocalizedString(@"Manual", nil);
    } else if([anObject isEqualToNumber:kPayoutManual]) {
        return NSLocalizedString(@"Depends on Turnout", nil);
    } else {
        return NSLocalizedString(@"Unknown Payout Policy", nil);
    }
}

- (BOOL)getObjectValue:(id*)obj forString:(NSString*)string errorDescription:(NSString**)error {
    NSLog(@"Unexpected call to getObjectValue:forString:errorDescription:");
    return NO;
}

@end
