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

@interface TBSeatingChartViewController () <NSCollectionViewDataSource, NSCollectionViewDelegateFlowLayout>

// array controller for objects managed by this view controller
@property (nonatomic, strong) IBOutlet NSCollectionView* collectionView;

// text color
@property (nonatomic, strong) TBColor* textColor;

// player lists per table
@property (nonatomic, strong) NSMutableDictionary* tables;

@end

@implementation TBSeatingChartViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // alloc colors
    _textColor = [TBColor labelColor];

    // alloc tables
    _tables = [NSMutableDictionary dictionary];

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
    [[self KVOController] observe:self keyPaths:@[@"session.state.seating_chart"] options:NSKeyValueObservingOptionInitial block:^(id observer, TBSeatingChartViewController* object, NSDictionary *change) {

        // delete existing tables and re-create
        [[self tables] removeAllObjects];

        // iterate through list to build up our table_name keyed dictionary
        NSArray* allPlayers = [[self session] state][@"seating_chart"];
        for(NSDictionary* seatingChartEntry in allPlayers) {
            // given this player's table
            NSString* tableName = seatingChartEntry[@"table_name"];
            if(tableName != nil) {
                // look up the seating list
                NSMutableArray* seats = [[self tables] objectForKey:tableName];

                // create if needed
                if(seats == nil) {
                    seats = [NSMutableArray array];

                    // set it into our dictionary
                    [[self tables] setObject:seats forKey:tableName];
                }

                // now add the seat
                [seats addObject:seatingChartEntry];
            }
        }

        // reload collectionView
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
    TBSeatingChartCollectionViewItem* item = (TBSeatingChartCollectionViewItem*)[[self collectionView] makeItemWithIdentifier:@"TBSeatingChartCollectionViewItem" forIndexPath:indexPath];

    if([indexPath section] == 0) {

        // compare vs expected number of tables
        NSInteger tableCount = [[[self session] state][@"table_count"] integerValue];
        if([indexPath item] < tableCount) {
            // get table name for this item
            NSArray* tablesPlaying = [[self session] state][@"tables_playing"];
            NSUInteger uitem = (NSUInteger)[indexPath item];
            NSString* tableName = tablesPlaying[uitem];

            // set item attributes
            [item setTableName:tableName];
            [item setSeats:[[self tables] objectForKey:tableName]];

        } else {
            NSLog(@"TBSeatingChartViewController collectionView:itemForRepresentedObjectAtIndexPath: requested item %zd but table_count is %zd",
                  [indexPath item], tableCount );
        }
    } else {
        NSLog(@"TBSeatingChartViewController collectionView:itemForRepresentedObjectAtIndexPath: invalid section");
    }

    return item;
}

- (NSInteger)collectionView:(NSCollectionView*)collectionView numberOfItemsInSection:(NSInteger)section {
    NSInteger tableCount = [[[self session] state][@"table_count"] integerValue];
    return tableCount;
}

#pragma mark NSCollectionViewDelegateFlowLayout

- (NSSize)collectionView:(NSCollectionView*)collectionView layout:(NSCollectionViewLayout*)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath*)indexPath {
    // default size
    NSSize size = NSMakeSize(320.0f, 108.0f);

    // get table name for this item
    NSArray* tablesPlaying = [[self session] state][@"tables_playing"];
    NSUInteger uitem = (NSUInteger)[indexPath item];
    NSString* tableName = tablesPlaying[uitem];

    // look up seats
    if(tableName != nil) {
        // look up the seating list
        NSArray* seats = [[self tables] objectForKey:tableName];

        // if it exists, calculate height
        if(seats != nil) {
            size.height = 20.0f + 35.0f + 8.0f + 25.0f + 35.0f * [seats count] + 20.0f;
        }
    }

    return size;
}


@end
