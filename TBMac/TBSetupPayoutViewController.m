//
//  TBSetupPayoutViewController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSetupPayoutViewController.h"
#import "NSObject+FBKVOController.h"
#import "TournamentSession.h"
#import <Foundation/Foundation.h>

// TBSetupPayoutArrayController implements a new object
@interface TBSetupPayoutArrayController : NSArrayController

@end

@implementation TBSetupPayoutArrayController

- (id)newObject {
    NSNumber* amount = @0;
    return [@{@"amount":amount} mutableCopy];
}

@end

@interface TBSetupPayoutViewController () <NSTableViewDataSource>

@end

@implementation TBSetupPayoutViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // observe change to payout currency
    [[self KVOController] observe:self keyPath:@"representedObject.payout_currency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        // refresh
        [[self tableView] reloadData];
    }];
}

#pragma mark NSTableViewDataSource

- (id)tableView:(NSTableView*)aTableView objectValueForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    if([[aTableColumn identifier] isEqualToString:@"Place"]) {
        return @(rowIndex+1);
    }
    return nil;
}

@end
