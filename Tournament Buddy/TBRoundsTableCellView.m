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
    }
}

- (void)setRoundNumber:(NSInteger)round {
    [[self textField] setStringValue:[NSString stringWithFormat:@"%li",(long)round]];
}

#pragma mark Controls

- (IBAction)breakButtonDidChange:(id)sender {
    NSButton* button = sender;
    BOOL isBreak = [button state] == NSOnState;
    [[self breakDurationField] setHidden:!isBreak];
    [[self breakMessageButton] setHidden:!isBreak];
    [[self breakLabel] setHidden:!isBreak];
}

@end
