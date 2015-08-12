//
//  TBAuthsViewController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBAuthsViewController.h"

@implementation TBAuthsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* nameSort = [[NSSortDescriptor alloc] initWithKey:@"added_on" ascending:YES];
    [[self arrayController] setSortDescriptors:@[nameSort]];
}

@end
