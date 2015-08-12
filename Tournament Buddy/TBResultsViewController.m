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

// TBResultsTableCellView to simply handle the break button check box
@interface TBResultsTableCellView : NSTableCellView

@property (weak) IBOutlet NSTextField* placeTextField;
@property (weak) IBOutlet NSTextField* payoutTextField;

@end

@implementation TBResultsTableCellView

@end

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
    TBResultsTableCellView* result = [aTableView makeViewWithIdentifier:aTableColumn.identifier owner:self];
    [[result payoutTextField] setFormatter:[self equityFormatter]];
    return result;
}

@end
