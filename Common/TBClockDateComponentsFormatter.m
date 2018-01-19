//
//  TBClockDateComponentsFormatter.m
//  td
//
//  Created by Ryan Drake on 1/19/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBClockDateComponentsFormatter.h"

@implementation TBClockDateComponentsFormatter

- (NSString*)stringFromMillisecondsRemaining:(NSNumber*)clockRemaining atMillisecondsSince1970:(NSNumber*)currentTime running:(NSNumber*)running {
    if([running boolValue]) {
        // calculate difference between this device's clock and remote clock
        NSDate* localDate = [NSDate date];
        long long localNow = [localDate timeIntervalSince1970] * 1000;
        long long remoteNow = [currentTime longLongValue];
        long long offset = remoteNow - localNow;

        // apply as offset to given clock
        long long milliseconds = [clockRemaining longLongValue];
        long long displayMilliseconds = milliseconds + offset;

        return [self stringFromTimeInterval:displayMilliseconds/1000.0];
    } else {
        // clock is paused
        return NSLocalizedString(@"PAUSED", nil);
    }
}

@end
