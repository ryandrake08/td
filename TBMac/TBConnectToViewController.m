//
//  TBConnectToViewController.m
//  td
//
//  Created by Ryan Drake on 8/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBConnectToViewController.h"
#import "TournamentSession.h"

@implementation TBConnectToViewController

#pragma mark Actions

- (IBAction)connectButtonDidChange:(id)sender {
    TournamentService* service = [[TournamentService alloc] initWithAddress:[self address] andPort:[self port]];
    NSError* error;
    if(![[self session] connectToTournamentService:service error:&error]) {
        [self presentError:error];
    }

    [self dismissViewController:self];
}

@end
