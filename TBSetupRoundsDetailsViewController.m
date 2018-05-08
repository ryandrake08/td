//
//  TBSetupRoundsDetailsViewController.m
//  TBMac
//
//  Created by Ryan Drake on 5/4/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupRoundsDetailsViewController.h"
#import "TournamentSession.h"

@interface TBSetupRoundsDetailsViewController ()

// Array controller for objects managed by this view controller
@property (nonatomic, strong) IBOutlet NSObjectController *requestObjectController;

@end

@implementation TBSetupRoundsDetailsViewController

- (IBAction)doneButtonDidChange:(id)sender {
    NSDictionary* request = [[self requestObjectController] content];
    [[self session] genBlindLevelsRequest:request withBlock:^(NSArray* returnedLevels) {
        // set configuration
        [[self configuration] setValue:returnedLevels forKey:@"blind_levels"];
    }];

    // dismiss
    [self dismissController:self];
}

@end
