//
//  TBPlayerWindowController.m
//  td
//
//  Created by Ryan Drake on 6/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBPlayerWindowController.h"
#import "TBActionClockView.h"
#import "TBResizeTextField.h"
#import "TBStatsViewController.h"
#import "TBResultsViewController.h"
#import "TBChipsDisplayViewController.h"
#import "TBControlsViewController.h"
#import "TBSound.h"
#import "NSObject+FBKVOController.h"

@interface TBPlayerWindowController ()

// View controllers
@property (strong) IBOutlet TBStatsViewController* statsViewController;
@property (strong) IBOutlet TBResultsViewController* resultsViewController;
@property (strong) IBOutlet TBChipsDisplayViewController* chipsDisplayViewController;
@property (strong) IBOutlet TBControlsViewController* controlsViewController;

// UI elements
@property (weak) IBOutlet NSImageView* backgroundImageView;
@property (weak) IBOutlet TBActionClockView* actionClockView;
@property (weak) IBOutlet NSView* leftPaneView;
@property (weak) IBOutlet NSView* rightPaneView;
@property (weak) IBOutlet NSView* controlsView;
@property (weak) IBOutlet NSView* leftAccessoryView;

// Sounds
@property (strong) TBSound* startSound;
@property (strong) TBSound* nextSound;
@property (strong) TBSound* breakSound;
@property (strong) TBSound* warningSound;

@end

@implementation TBPlayerWindowController

- (void)windowDidLoad {
    [super windowDidLoad];
    
    // register for KVO
    [[self KVOController] observe:[[self session] state] keyPath:@"current_blind_level" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
        id old = change[@"old"];
        id new = change[@"new"];
        if(![old isEqualTo:[NSNull null]] && ![new isEqualTo:[NSNull null]]) {
            if([old isEqualTo:@0] && ![new isEqualTo:@0]) {
                [[self startSound] play];
            } else if(![old isEqualTo:@0] && [new isEqualTo:@0]) {
                // restart game
            } else if (![old isEqualTo:[NSNull null]] && ![old isEqualTo:new]) {
                [[self nextSound] play];
            }
        }
    }];

    [[self KVOController] observe:[[self session] state] keyPaths:@[@"on_break"] options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
        id old = change[@"old"];
        id new = change[@"new"];
        if(![old isEqualTo:[NSNull null]] && ![new isEqualTo:[NSNull null]]) {
            if([old isEqualTo:@NO] && [new isEqualTo:@YES]) {
                [[self breakSound] play];
            }
        }
    }];

    [[self KVOController] observe:[[self session] state] keyPaths:@[@"time_remaining",@"break_time_remaining"] options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
        id old = change[@"old"];
        id new = change[@"new"];
        if(![old isEqualTo:[NSNull null]] && ![new isEqualTo:[NSNull null]]) {
            if([old integerValue] > kAudioWarningTime && [new integerValue] <= kAudioWarningTime && [new integerValue] != 0) {
                [[self warningSound] play];
            }
        }
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"action_clock_time_remaining" options:0 block:^(id observer, id object, NSDictionary *change) {
        NSUInteger actionClockTimeRemaining = [object[@"action_clock_time_remaining"] unsignedIntegerValue];
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }];

    // alloc sounds
    [self setStartSound:[[TBSound alloc] initWithResource:@"s_start" extension:@"caf"]];
    [self setNextSound:[[TBSound alloc] initWithResource:@"s_next" extension:@"caf"]];
    [self setBreakSound:[[TBSound alloc] initWithResource:@"s_break" extension:@"caf"]];
    [self setWarningSound:[[TBSound alloc] initWithResource:@"s_warning" extension:@"caf"]];

    // add subivews
    [[self statsViewController] setSession:[self session]];
    [[self leftPaneView] addSubview:[[self statsViewController] view]];
    [[self chipsDisplayViewController] setSession:[self session]];
    [[self leftAccessoryView] addSubview:[[self chipsDisplayViewController] view]];
    [[self resultsViewController] setSession:[self session]];
    [[self rightPaneView] addSubview:[[self resultsViewController] view]];
    [[self controlsViewController] setSession:[self session]];
    [[self controlsView] addSubview:[[self controlsViewController] view]];
}

#pragma mark TBActionClockViewDelegate

- (CGFloat)analogClock:(TBActionClockView*)clock graduationLengthForIndex:(NSInteger)index {
    return index % 5 == 0 ? 10.0 : 5.0;
}

@end
