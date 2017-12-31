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

@end

@implementation TBPlayerWindowController

- (void)windowDidLoad {
    [super windowDidLoad];
    
    [[self KVOController] observe:self keyPath:@"session.state.action_clock_time_remaining" options:0 block:^(id observer, TBPlayerWindowController* object, NSDictionary *change) {
        NSUInteger actionClockTimeRemaining = [[[object session] state][@"action_clock_time_remaining"] unsignedIntegerValue];
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }];

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
