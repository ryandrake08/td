//
//  TBPlayerWindowController.m
//  td
//
//  Created by Ryan Drake on 6/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBPlayerWindowController.h"

#import "NSObject+FBKVOController.h"

@interface TBPlayerWindowController ()

@property (weak) IBOutlet NSImageView* backgroundImageView;
@property (weak) IBOutlet TBResizeTextField* tournamentNameLabel;
@property (weak) IBOutlet TBResizeTextField* tournamentFundingLabel;
@property (weak) IBOutlet TBResizeTextField* clockLabel;
@property (weak) IBOutlet TBResizeTextField* currentGameLabel;
@property (weak) IBOutlet TBResizeTextField* currentRoundLabel;
@property (weak) IBOutlet TBResizeTextField* nextGameLabel;
@property (weak) IBOutlet TBResizeTextField* nextRoundLabel;
@property (weak) IBOutlet TBResizeTextField* currentRoundNumberLabel;
@property (weak) IBOutlet TBResizeTextField* playersLeftLabel;
@property (weak) IBOutlet TBResizeTextField* averageStackLabel;
@property (weak) IBOutlet NSButton* previousRoundButton;
@property (weak) IBOutlet NSButton* pauseResumeButton;
@property (weak) IBOutlet NSButton* nextRoundButton;
@property (weak) IBOutlet NSButton* callClockButton;

@end

@implementation TBPlayerWindowController

#define kActionClockRequestTime 60000

- (void)windowDidLoad {
    [super windowDidLoad];
    
    // register for KVO
    [[self KVOController] observe:[self session] keyPaths:@[@"isConnected", @"authorized"] options:0 block:^(id observer, id object, NSDictionary *change) {
        [observer updateButtons];
    }];

    [[self KVOController] observe:[self session] keyPath:@"currentBlindLevel" options:0 block:^(id observer, id object, NSDictionary *change) {
        [observer updateButtons];
        if([[object currentBlindLevel] isEqualToNumber:[NSNumber numberWithInt:0]]) {
            [[observer currentRoundNumberLabel] setStringValue:@"-"];
        } else {
           [[observer currentRoundNumberLabel] setStringValue:[NSString stringWithFormat:@"%@", [object currentBlindLevel]]];
        }
    }];

    [[self KVOController] observe:[self session] keyPath:@"clockText" options:0 block:^(id observer, id object, NSDictionary *change) {
        [[observer clockLabel] setStringValue:[object clockText]];
    }];

    [[self KVOController] observe:[self session] keyPath:@"currentGameText" options:0 block:^(id observer, id object, NSDictionary *change) {
        [[observer currentGameLabel] setStringValue:[object currentGameText]];
    }];

    [[self KVOController] observe:[self session] keyPath:@"currentRoundText" options:0 block:^(id observer, id object, NSDictionary *change) {
        [[observer currentRoundLabel] setStringValue:[object currentRoundText]];
    }];

    [[self KVOController] observe:[self session] keyPath:@"nextGameText" options:0 block:^(id observer, id object, NSDictionary *change) {
        [[observer nextGameLabel] setStringValue:[object nextGameText]];
    }];
    
    [[self KVOController] observe:[self session] keyPath:@"nextRoundText" options:0 block:^(id observer, id object, NSDictionary *change) {
        [[observer nextRoundLabel] setStringValue:[object nextRoundText]];
    }];

    [[self KVOController] observe:[self session] keyPath:@"playersLeftText" options:0 block:^(id observer, id object, NSDictionary *change) {
        [[observer playersLeftLabel] setStringValue:[object playersLeftText]];
    }];

    [[self KVOController] observe:[self session] keyPath:@"averageStackText" options:0 block:^(id observer, id object, NSDictionary *change) {
        [[observer averageStackLabel] setStringValue:[object averageStackText]];
    }];

    [[self KVOController] observe:[self session] keyPath:@"actionClockTimeRemaining" options:0 block:^(id observer, id object, NSDictionary *change) {
        [observer updateActionClock];
    }];
}

#pragma mark Update

- (void)updateButtons {
    BOOL connected = [[self session] isConnected];
    BOOL authorized = [[self session] isAuthorized];
    if(connected && authorized) {
        [[self previousRoundButton] setHidden:NO];
        [[self pauseResumeButton] setHidden:NO];
        [[self nextRoundButton] setHidden:NO];
        [[self callClockButton] setHidden:NO];
    } else {
        [[self previousRoundButton] setHidden:YES];
        [[self pauseResumeButton] setHidden:YES];
        [[self nextRoundButton] setHidden:YES];
        [[self callClockButton] setHidden:YES];
    }

    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel == 0) {
        [[self previousRoundButton] setEnabled:NO];
        [[self pauseResumeButton] setEnabled:YES];
        [[self nextRoundButton] setEnabled:NO];
        [[self callClockButton] setEnabled:NO];
    } else {
        [[self previousRoundButton] setEnabled:YES];
        [[self pauseResumeButton] setEnabled:YES];
        [[self nextRoundButton] setEnabled:YES];
        [[self callClockButton] setEnabled:YES];
    }
}

- (void)updateActionClock {
#if 0
    NSUInteger actionClockTimeRemaining = [[[self session] actionClockTimeRemaining] unsignedIntegerValue];
    if(actionClockTimeRemaining == 0) {
        [[self actionClockView] setHidden:YES];
    } else {
        [[self actionClockView] setHidden:NO];
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }
#endif
}

#pragma mark Actions

- (IBAction)previousRoundTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGameAt:nil];
    }
}

- (IBAction)nextRoundTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        NSUInteger remaining = [[[self session] actionClockTimeRemaining] unsignedIntegerValue];
        if(remaining == 0) {
            [[self session] setActionClock:[NSNumber numberWithUnsignedInteger:kActionClockRequestTime]];
        } else {
            [[self session] setActionClock:nil];
        }
    }
}

@end
