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
#import "NSObject+FBKVOController.h"

@interface TBPlayerWindowController ()

// View controllers
@property (strong) IBOutlet TBResultsViewController* resultsViewController;
@property (strong) IBOutlet TBControlsViewController* controlsViewController;

// UI elements
@property (weak) IBOutlet NSImageView* backgroundImageView;
@property (weak) IBOutlet TBActionClockView* actionClockView;
@property (weak) IBOutlet NSView* rightPaneView;
@property (weak) IBOutlet NSView* controlsView;

@end

@implementation TBPlayerWindowController

- (void)windowDidLoad {
    [super windowDidLoad];
    
    // register for KVO
    [[self KVOController] observe:[self session] keyPath:@"actionClockTimeRemaining" options:0 block:^(id observer, id object, NSDictionary *change) {
        NSUInteger actionClockTimeRemaining = [[[self session] actionClockTimeRemaining] unsignedIntegerValue];
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }];

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
