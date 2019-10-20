//
//  TBSeatingChartCollectionViewItem.m
//  td
//
//  Created by Ryan Drake on 10/14/19.
//  Copyright © 2019 HDna Studio. All rights reserved.
//

#import "TBSeatingChartCollectionViewItem.h"

@interface TBSeatingChartCollectionViewItem ()

// UI Outlets
@property (nonatomic, weak) IBOutlet NSTableView* tableView;

// array controller for objects managed by this view controller
@property (nonatomic, strong) IBOutlet NSArrayController* arrayController;

@end

@implementation TBSeatingChartCollectionViewItem

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* seatNumberSort = [[NSSortDescriptor alloc] initWithKey:@"seat_name" ascending:YES comparator:^NSComparisonResult(id obj1, id obj2) {
        return [obj1 compare:obj2 options:NSCaseInsensitiveSearch|NSNumericSearch];
    }];

    // set sort descriptors for arrays
    [[self arrayController] setSortDescriptors:@[seatNumberSort]];

    // resize columns
    [[self tableView] sizeToFit];
}

- (void)setTableName:(NSString*)tableName {
    _tableName = [tableName copy];
    
    // filter predicate to not show empty seats
    [[self arrayController] setFilterPredicate:[NSPredicate predicateWithFormat: @"table_name == %@ AND seat_name != nil", tableName]];
}

@end
