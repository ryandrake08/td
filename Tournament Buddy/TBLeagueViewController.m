//
//  TBLeagueViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBLeagueViewController.h"

@implementation TBLeagueViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* nameSort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES];
    [[self arrayController] setSortDescriptors:@[nameSort]];
}

@end
