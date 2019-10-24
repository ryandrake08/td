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

@end

@implementation TBSeatingChartViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // alloc colors
    _textColor = [TBColor labelColor];

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
    [[self KVOController] observe:self keyPaths:@[@"session.state.seated_players", @"session.state.empty_seated_players"] options:NSKeyValueObservingOptionInitial block:^(id observer, TBSeatingChartViewController* object, NSDictionary *change) {
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
    TBSeatingChartCollectionViewItem* item;

    if([indexPath section] == 0) {

        // compare vs expected number of tables
        NSInteger tableCount = [[[self session] state][@"table_count"] integerValue];
        if([indexPath item] < tableCount) {
            item = (TBSeatingChartCollectionViewItem*)[[self collectionView] makeItemWithIdentifier:@"TBSeatingChartCollectionViewItem" forIndexPath:indexPath];

            // get table name for this item
            NSArray* tablesPlaying = [[self session] state][@"tables_playing"];
            NSString* tableName = tablesPlaying[[indexPath item]];

            // build up list of seats for this table
            NSMutableArray* seats = [[NSMutableArray alloc] init];

            // iterate through seated player list to find all seats with correct table name
            NSArray* seatedPlayers = [[self session] state][@"seated_players"];
            for(NSDictionary* seatedPlayer in seatedPlayers) {
                if([tableName isEqualToString:seatedPlayer[@"table_name"]]) {
                    [seats addObject:seatedPlayer];
                }
            }

            // iterate through empty seats adding blank seats
            NSArray* emptySeats = [[self session] state][@"empty_seated_players"];
            for(NSDictionary* emptySeat in emptySeats) {
                if([tableName isEqualToString:emptySeat[@"table_name"]]) {
                    [seats addObject:emptySeat];
                }
            }

            // set item attributes
            [item setTableName:tableName];
            [item setSeats:seats];

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

@end
