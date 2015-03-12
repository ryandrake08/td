//
//  TBRoundsTableCellView.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 3/8/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRoundsTableCellView.h"

@interface TBRoundsTableCellView ()

@property IBOutlet NSTextField* durationField;
@property IBOutlet NSButton* breakButton;
@property IBOutlet NSTextField* breakDurationField;
@property IBOutlet NSButton* breakMessageButton;
@property IBOutlet NSTextField* breakLabel;

@end

@implementation TBRoundsTableCellView

- (void)setObjectValue:(id)objectValue {
    [super setObjectValue:objectValue];

    if([self objectValue]) {

        BOOL isBreak = [[[self objectValue] objectForKey:@"break_duration"] boolValue];
        [[self breakButton] setState:isBreak ? NSOnState : NSOffState];
        [[self breakDurationField] setHidden:!isBreak];
        [[self breakMessageButton] setHidden:!isBreak];
        [[self breakLabel] setHidden:!isBreak];

        // Handle duration
        NSNumber* durationNumber = [[self objectValue] objectForKey:@"duration"];
        if(durationNumber) {
            NSInteger duration = [durationNumber integerValue];
            NSInteger minutes = (duration / 60000) % 60;
            NSInteger hours = (duration / 3600000);
            NSString* value = [NSString stringWithFormat:@"%02ld:%02ld", (long)hours, (long)minutes];
            [[self durationField] setStringValue:value];
        }

        // Handle break duration
        NSNumber* breakDurationNumber = [[self objectValue] objectForKey:@"break_duration"];
        if(breakDurationNumber) {
            NSInteger duration = [breakDurationNumber integerValue];
            NSInteger minutes = (duration / 60000) % 60;
            NSInteger hours = (duration / 3600000);
            NSString* value = [NSString stringWithFormat:@"%02ld:%02ld", (long)hours, (long)minutes];
            [[self breakDurationField] setStringValue:value];
        }
    }
}

- (void)setRoundNumber:(NSInteger)round {
    [[self textField] setStringValue:[NSString stringWithFormat:@"%li",(long)round]];
}

#pragma mark Controls

+ (NSNumber*)durationForTextField:(NSTextField*)field {
    NSString* value = [field stringValue];
    NSScanner* scanner = [NSScanner scannerWithString:value];
    NSInteger hours, minutes;
    [scanner scanInteger:&hours];
    [scanner scanString:@":" intoString:NULL];
    [scanner scanInteger:&minutes];
    NSInteger duration = hours * 3600000 + minutes * 60000;
    return [NSNumber numberWithInteger:duration];
}

- (IBAction)durationFieldDidChange:(id)sender {
    NSNumber* number = [[self class] durationForTextField:sender];
    [[self objectValue] setObject:number forKey:@"duration"];
}

- (IBAction)breakDurationFieldDidChange:(id)sender {
    NSNumber* number = [[self class] durationForTextField:sender];
    [[self objectValue] setObject:number forKey:@"break_duration"];
}

- (IBAction)breakButtonDidChange:(id)sender {
    NSButton* button = sender;
    BOOL isBreak = [button state] == NSOnState;
    [[self breakDurationField] setHidden:!isBreak];
    [[self breakMessageButton] setHidden:!isBreak];
    [[self breakLabel] setHidden:!isBreak];
}

@end
