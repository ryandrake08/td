//
//  TBPlanViewController.m
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBPlanViewController.h"
#import "TBMacDocument.h"
#import "TBNotifications.h"
#import "TournamentSession.h"

@interface TBPlanViewController ()

// UI Outlets
@property (nonatomic, strong) IBOutlet NSTextField* playersTextField;

@end

@implementation TBPlanViewController

#pragma mark Actions

- (void)viewDidLoad {
    [super viewDidLoad];

    // set default value for text field
    [[self playersTextField] setIntegerValue:(NSInteger)[self numberOfPlayers]];
}

- (IBAction)doneButtonDidChange:(id)sender {
    NSInteger players = [[self playersTextField] integerValue];
    if(players != 0) {
        // plan seating
        [[self session] planSeatingFor:@(players) withBlock:^(NSArray* movements) {
            if([movements count] > 0) {
                [[NSNotificationCenter defaultCenter] postNotificationName:kMovementsUpdatedNotification object:movements];
                [self performSegueWithIdentifier:@"presentMovementView" sender:sender];
            }
        }];

    }

    // dismiss
    [self dismissController:self];
}

@end
