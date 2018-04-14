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
#import "TournamentSession.h"

@implementation TBResultsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptor
    NSSortDescriptor* placeSort = [[NSSortDescriptor alloc] initWithKey:@"place" ascending:YES];
    [[self arrayController] setSortDescriptors:@[placeSort]];
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView*)aTableView viewForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    NSTableCellView* result = [aTableView makeViewWithIdentifier:aTableColumn.identifier owner:self];
    if([[aTableColumn identifier] isEqualToString:@"Payout"]) {
        NSDictionary* object = [[[self arrayController] arrangedObjects] objectAtIndex:rowIndex];
        [[[result textField] formatter] setCurrencyCode:object[@"payout"][@"currency"]];
    }
    return result;
}

@end
