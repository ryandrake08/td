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
#import "TBResultsViewController.h"
#import "TBControlsViewController.h"
#import "TBSound.h"
#import "NSObject+FBKVOController.h"

@interface TBPlayerWindowController ()

#define kWarningTime 60000

// View controllers
@property (strong) IBOutlet TBResultsViewController* resultsViewController;
@property (strong) IBOutlet TBControlsViewController* controlsViewController;

// UI elements
@property (weak) IBOutlet NSImageView* backgroundImageView;
@property (weak) IBOutlet TBActionClockView* actionClockView;
@property (weak) IBOutlet NSView* rightPaneView;
@property (weak) IBOutlet NSView* controlsView;

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
    [[self KVOController] observe:[self session] keyPath:NSStringFromSelector(@selector(currentBlindLevel)) options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
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

    [[self KVOController] observe:[self session] keyPaths:@[@"onBreak"] options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
        id old = change[@"old"];
        id new = change[@"new"];
        if(![old isEqualTo:[NSNull null]] && ![new isEqualTo:[NSNull null]]) {
            if([old isEqualTo:@NO] && [new isEqualTo:@YES]) {
                [[self breakSound] play];
            }
        }
    }];

    [[self KVOController] observe:[self session] keyPaths:@[@"timeRemaining",@"breakTimeRemaining"] options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
        id old = change[@"old"];
        id new = change[@"new"];
        if(![old isEqualTo:[NSNull null]] && ![new isEqualTo:[NSNull null]]) {
            if([old integerValue] > kWarningTime && [new integerValue] <= kWarningTime && [new integerValue] != 0) {
                [[self warningSound] play];
            }
        }
    }];

    [[self KVOController] observe:[self session] keyPath:@"actionClockTimeRemaining" options:0 block:^(id observer, id object, NSDictionary *change) {
        NSUInteger actionClockTimeRemaining = [[[self session] actionClockTimeRemaining] unsignedIntegerValue];
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }];

    // alloc sounds
    [self setStartSound:[[TBSound alloc] initWithResource:@"s_start" extension:@"caf"]];
    [self setNextSound:[[TBSound alloc] initWithResource:@"s_next" extension:@"caf"]];
    [self setBreakSound:[[TBSound alloc] initWithResource:@"s_break" extension:@"caf"]];
    [self setWarningSound:[[TBSound alloc] initWithResource:@"s_warning" extension:@"caf"]];

    // add subivews
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
