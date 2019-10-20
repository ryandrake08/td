//
//  TBSeatingChartViewController.m
//  td
//
//  Created by Ryan Drake on 10/13/19.
//  Copyright © 2019 HDna Studio. All rights reserved.
//

#import "TBSeatingChartViewController.h"
#import "TBSeatingChartCollectionViewItem.h"
#import "NSObject+FBKVOController.h"
#import "TBColor+ContrastTextColor.h"
#import "TBColor+CSS.h"
#import "TournamentSession.h"

@interface TBSeatingChartViewController () <NSCollectionViewDataSource>

// array controller for objects managed by this view controller
@property (nonatomic, strong) IBOutlet NSCollectionView* collectionView;

// Text color
@property (nonatomic, strong) TBColor* textColor;

// Cache list of table names
@property (nonatomic, strong) NSMutableArray* tableNames;

@end

@implementation TBSeatingChartViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // alloc colors
    _textColor = [TBColor labelColor];

    // alloc tableNames
    _tableNames = [[NSMutableArray alloc] init];

    // update window title when tournament name changes
    [[self KVOController] observe:self keyPath:@"session.state.name" options:NSKeyValueObservingOptionInitial block:^(id observer, TBSeatingChartViewController* object, NSDictionary *change) {
        NSString* tournamentName = [[object session] state][@"name"];
        if(tournamentName == nil) {
            [[[self view] window] setTitle:NSLocalizedString(@"Tournament Display", nil)];
        } else {
            [[[self view] window] setTitle:[NSString localizedStringWithFormat:@"Display: %@", tournamentName]];
        }
    }];

    // update chart when seating changes
    [[self KVOController] observe:self keyPaths:@[@"session.state.seated_players"] options:NSKeyValueObservingOptionInitial block:^(id observer, TBSeatingChartViewController* object, NSDictionary *change) {
        // iterate throuhg list to find all table names
        [[self tableNames] removeAllObjects];
        for (NSDictionary* seatedPlayer in [[object session] state][@"seated_players"]) {
            NSString* tableName = seatedPlayer[@"table_name"];
            if([tableName length] && ![[self tableNames] containsObject:tableName]) {
                [[self tableNames] addObject:tableName];
            }
        }
        // reload
        [[self collectionView] reloadData];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.background_color" options:NSKeyValueObservingOptionInitial block:^(id observer, TBSeatingChartViewController* object, NSDictionary *change) {
        // Set the background color on the view
        NSString* backgroundColorName = [[object session] state][@"background_color"];
        if(backgroundColorName != nil && ![backgroundColorName isEqualToString:@""]) {
            TBColor* color = [TBColor colorWithName:backgroundColorName];
            [[object view] setBackgroundColor:color];

            // All text fields in view are bound to this color. Set once, set for all
            [object setTextColor:[color contrastTextColor]];
        }
    }];
}

#pragma mark NSCollectionViewDataSource

- (NSCollectionViewItem*)collectionView:(NSCollectionView*)collectionView itemForRepresentedObjectAtIndexPath:(NSIndexPath*)indexPath {
    TBSeatingChartCollectionViewItem* item;

    if([indexPath section] == 0) {
        if([indexPath item] < [[self tableNames] count]) {
            item = (TBSeatingChartCollectionViewItem*)[[self collectionView] makeItemWithIdentifier:@"TBSeatingChartCollectionViewItem" forIndexPath:indexPath];

            // set session
            [item setSession:[self session]];

            // set table name
            [item setTableName:[self tableNames][[indexPath item]]];
        } else {
            NSLog(@"TBSeatingChartViewController collectionView:itemForRepresentedObjectAtIndexPath: requested item %zd but table_count is %@",
                  [indexPath item], [[self session] state][@"table_count"] );
        }
    } else {
        NSLog(@"TBSeatingChartViewController collectionView:itemForRepresentedObjectAtIndexPath: invalid section");
    }

    return item;
}

- (NSInteger)collectionView:(NSCollectionView*)collectionView numberOfItemsInSection:(NSInteger)section {
    return [[[self session] state][@"table_count"] integerValue];
}

@end
