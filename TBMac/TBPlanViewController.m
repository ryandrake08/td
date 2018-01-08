//
//  TBPlanViewController.m
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBPlanViewController.h"
#import "Document.h"
#import "NSObject+FBKVOController.h"
#import "TournamentSession.h"

@interface TBPlanViewController ()

// UI Outlets
@property (strong) IBOutlet NSTextField* playersTextField;
@property (strong) IBOutlet NSTextField* warningTextField;

@end

@implementation TBPlanViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // register for KVO
    [[self KVOController] observe:self keyPath:@"view.window.sheetParent.windowController.document" options:0 block:^(id observer, TBPlanViewController* object, NSDictionary *change) {
        // get document from sheet parent
        Document* document = [[[[[self view] window] sheetParent] windowController] document];

        // get max expected
        NSNumber* maxExpected = [[document session] state][@"max_expected_players"];

        // hide or show
        [[object warningTextField] setHidden:![maxExpected boolValue]];
    }];
}

#pragma mark Actions

- (IBAction)doneButtonDidChange:(id)sender {
    NSInteger players = [[self playersTextField] integerValue];
    if(players != 0) {
        // get document from sheet parent
        Document* document = [[[[[self view] window] sheetParent] windowController] document];

        // plan seating
        [document planSeatingFor:players];
    }

    // dismiss
    [self dismissController:self];
}

@end
