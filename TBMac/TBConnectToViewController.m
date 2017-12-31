//
//  TBConnectToViewController.m
//  td
//
//  Created by Ryan Drake on 8/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBConnectToViewController.h"

@implementation TBConnectToViewController

#pragma mark Actions

- (IBAction)connectButtonDidChange:(id)sender {
    if(![[self session] connectToAddress:[self address] port:[self port]]) {
        // TODO: handle error
    }

    [self dismissViewController:self];
}

@end
