//
//  TBNotificationAttributes.m
//  td
//
//  Created by Ryan Drake on 10/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBNotificationAttributes.h"

@implementation TBNotificationAttributes

// initialize this object from a tournament state
- (instancetype)initWithTournamentState:(NSDictionary*)state warningTime:(NSInteger)warningTime {
    if((self = [super init])) {
        // relevant state variables
        BOOL running = [state[@"running"] boolValue];
        NSInteger timeRemaining = [state[@"time_remaining"] integerValue];
        NSInteger breakTimeRemaining = [state[@"break_time_remaining"] integerValue];
        NSString* nextRoundText = state[@"next_round_text"];

        // set attributes if the clock is running
        if(running && (timeRemaining > 0 || breakTimeRemaining > 0)) {
            // use a date components formatter
            NSDateComponentsFormatter* formatter = [[NSDateComponentsFormatter alloc] init];
            [formatter setAllowedUnits:NSCalendarUnitHour | NSCalendarUnitMinute];
            [formatter setIncludesApproximationPhrase:NO];
            [formatter setIncludesTimeRemainingPhrase:YES];
            [formatter setMaximumUnitCount:2];
            [formatter setUnitsStyle:NSDateComponentsFormatterUnitsStyleSpellOut];
            [formatter setFormattingContext:NSFormattingContextBeginningOfSentence];

            NSTimeInterval interval = 0.0;

            // five possible notifications
            if(timeRemaining > warningTime) {
                // more than one minute of play left in round
                interval = (timeRemaining - warningTime) / 1000.0;
                _soundName = @"s_warning.caf";
                _body = [NSString localizedStringWithFormat:@"%@ in this round.", [formatter stringFromTimeInterval:warningTime/1000.0]];
                _title = NSLocalizedString(@"Warning", nil);
            } else if(timeRemaining > 0 && breakTimeRemaining > 0) {
                // less than one minute of play left in round with break coming up
                interval = timeRemaining / 1000.0;
                _soundName = @"s_break.caf";
                _body = [NSString localizedStringWithFormat:@"Players are now on break. %@.", [formatter stringFromTimeInterval:breakTimeRemaining/1000.0]];
                _title = NSLocalizedString(@"Break time", nil);
            } else if(timeRemaining > 0) {
                // less than one minute of play left in round and new round coming up
                interval = timeRemaining / 1000.0;
                _soundName = @"s_next.caf";
                _body = [NSString localizedStringWithFormat:@"New round: %@.", nextRoundText];
                _title = [nextRoundText copy];
            } else if(breakTimeRemaining > warningTime) {
                // more than one minute left in break
                interval = (breakTimeRemaining - warningTime) / 1000.0;
                _soundName = @"s_warning.caf";
                _body = [NSString localizedStringWithFormat:@"%@ until next round.", [formatter stringFromTimeInterval:warningTime/1000.0]];
                _title = NSLocalizedString(@"Warning", nil);
            } else if(breakTimeRemaining > 0) {
                // less than one minute left in break
                interval = breakTimeRemaining / 1000.0;
                _soundName = @"s_next.caf";
                _body = [NSString localizedStringWithFormat:@"New round: %@.", nextRoundText];
                _title = [nextRoundText copy];
            }

            // alert trigger date
            _date = [NSDate dateWithTimeIntervalSinceNow:interval];
        }

    }
    return self;
}

@end
