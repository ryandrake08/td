//
//  TBMovementViewController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBMovementViewController.h"

@interface TBMovementViewController ()

// ui
@property (strong) IBOutlet NSTextField* titleTextField;

// array controller for objects managed by this view controller
@property (strong) IBOutlet NSArrayController* arrayController;

@end

@implementation TBMovementViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    NSUInteger cellsToShow = [[[self arrayController] arrangedObjects] count];

    // change text based on whether we have movements
    if(cellsToShow == 0) {
        [[self titleTextField] setStringValue:NSLocalizedString(@"No players moving", nil)];
    } else if(cellsToShow == 1) {
        [[self titleTextField] setStringValue:NSLocalizedString(@"Move player:", nil)];
    } else {
        [[self titleTextField] setStringValue:NSLocalizedString(@"Move players:", nil)];
    }

    // size view content according to number of objects (max: 4)
    cellsToShow = cellsToShow > 4 ? 4 : cellsToShow;
    CGSize size = { 450.0f, cellsToShow * 40.0f + 116.0f };
    [self setPreferredContentSize:size];
}

@end
