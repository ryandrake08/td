//
//  TBResultsViewController.m
//  td
//
//  Created by Ryan Drake on 8/8/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBResultsViewController.h"
#import "NSObject+FBKVOController.h"
#import "TBCurrencyNumberFormatter.h"

@interface TBResultsViewController () <NSTableViewDelegate>

@property (strong) IBOutlet TBCurrencyNumberFormatter* equityFormatter;

@end

@implementation TBResultsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptor
    NSSortDescriptor* placeSort = [[NSSortDescriptor alloc] initWithKey:@"place" ascending:YES];
    [[self arrayController] setSortDescriptors:@[placeSort]];

    // register for KVO
    [[[self equityFormatter] KVOController] observe:[self session] keyPath:@"equityCurrency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [observer setCurrencyCode:[object equityCurrency]];
        [[self tableView] reloadData];
    }];
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView*)aTableView viewForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    NSTableCellView* result = [aTableView makeViewWithIdentifier:aTableColumn.identifier owner:self];
    if([[aTableColumn identifier] isEqualToString:@"Payout"]) {
        [[result textField] setFormatter:[self equityFormatter]];
    }
    return result;
}

@end
