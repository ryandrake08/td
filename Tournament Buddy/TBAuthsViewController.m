//
//  TBAuthsViewController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBAuthsViewController.h"
#import "NSObject+FBKVOController.h"

@implementation TBAuthsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* nameSort = [[NSSortDescriptor alloc] initWithKey:@"added_on" ascending:YES];
    [[self arrayController] setSortDescriptors:@[nameSort]];

    // register for KVO on arrangedObjects
    NSArray* keyPaths = @[@"arrangedObjects"];
    [[self KVOController] observe:[self arrayController] keyPaths:keyPaths options:0 block:^(id observer, id object, NSDictionary *change) {
        [[self session] selectiveConfigureAndUpdate:[self configuration]];
    }];
}

@end
