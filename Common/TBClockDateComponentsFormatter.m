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
        NSTimeInterval localNow = [[NSDate date] timeIntervalSince1970];
        NSTimeInterval remoteNow = [currentTime doubleValue]/1000.0;
        NSTimeInterval offset = remoteNow - localNow;

        // apply as offset to given clock
        NSTimeInterval displayTime = [clockRemaining doubleValue]/1000.0 + offset + 1.0;

        // format this timeInterval
        return [self stringFromTimeInterval:displayTime];
    } else {
        // clock is paused
        return NSLocalizedString(@"PAUSED", nil);
    }
}

@end
