//
//  TBPlanViewController.m
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBPlanViewController.h"
#import "TBMacDocument.h"
#import "TournamentSession.h"

@interface TBPlanViewController ()

// UI Outlets
@property (strong) IBOutlet NSTextField* playersTextField;

@end

@implementation TBPlanViewController

#pragma mark Actions

- (IBAction)doneButtonDidChange:(id)sender {
    NSInteger players = [[self playersTextField] integerValue];
    if(players != 0) {
        // session is represented object
        TournamentSession* session = [self representedObject];

        // plan seating
        [session planSeatingFor:@(players)];
    }

    // dismiss
    [self dismissController:self];
}

@end
