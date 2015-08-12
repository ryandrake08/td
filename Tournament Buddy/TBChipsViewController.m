//
//  TBChipsViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBChipsViewController.h"
#import "NSObject+FBKVOController.h"

@implementation TBChipsViewController

- (void)viewDidLoad {
    if([NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [super viewDidLoad];
    }

    // setup sort descriptors
    NSSortDescriptor* denominationSort = [[NSSortDescriptor alloc] initWithKey:@"denomination" ascending:YES];

    // set sort descriptors for arrays
    [[self arrayController] setSortDescriptors:@[denominationSort]];

    // register for KVO
    [[self KVOController] observe:[self configuration] keyPath:@"table_capacity" options:0 block:^(id observer, id object, NSDictionary *change) {
        [[self session] selectiveConfigureAndUpdate:[self configuration]];
    }];
}

- (void)loadView {
    [super loadView];
    if(![NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [self viewDidLoad];
    }
}

@end
