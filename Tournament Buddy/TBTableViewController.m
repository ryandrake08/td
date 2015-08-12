//
//  TBTableViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/31/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTableViewController.h"
#import "NSObject+FBKVOController.h"

@implementation TBTableViewController

- (void)viewDidLoad {
    if([NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [super viewDidLoad];
    }

    // register for KVO
    NSArray* keyPaths = @[@"arrangedObjects"];
    [[self KVOController] observe:[self arrayController] keyPaths:keyPaths options:0 block:^(id observer, id object, NSDictionary *change) {
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
