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
    NSError* error;
    if(![[self session] connectToAddress:[self address] port:[self port] error:&error]) {
        [self presentError:error];
    }

    [self dismissViewController:self];
}

@end
