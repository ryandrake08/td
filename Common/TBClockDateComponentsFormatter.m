//
//  TBClockDateComponentsFormatter.m
//  td
//
//  Created by Ryan Drake on 1/19/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBClockDateComponentsFormatter.h"

@implementation TBClockDateComponentsFormatter

- (NSString*)stringFromMillisecondsRemaining:(NSNumber*)clockRemaining atMillisecondsSince1970:(NSNumber*)currentTime countingDown:(BOOL)countingDown {
    // calculate difference between this device's clock and remote clock
    NSTimeInterval localNow = [[NSDate date] timeIntervalSince1970];
    NSTimeInterval remoteNow = [currentTime doubleValue]/1000.0;
    NSTimeInterval offset = countingDown ? remoteNow - localNow : localNow - remoteNow;

    // apply as offset to given clock
    NSTimeInterval displayTime = ceil([clockRemaining doubleValue]/1000.0 + offset);

    // format this timeInterval
    return [self stringFromTimeInterval:displayTime];
}

@end
