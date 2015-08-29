//
//  TBPlayerSeatingViewController.m
//  td
//
//  Created by Ryan Drake on 8/28/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBPlayerSeatingViewController.h"
#import "TournamentSession.h"
#import "TBAppDelegate.h"

#import "NSObject+FBKVOController.h"

@interface TBPlayerSeatingViewController () <UITableViewDelegate,
                                             UITableViewDataSource,
                                             UIActionSheetDelegate,
                                             UIAlertViewDelegate>

@property (nonatomic, strong) TournamentSession* session;

// tableview content
@property (nonatomic, strong) NSArray* seatedPlayers;
@property (nonatomic, strong) NSArray* unseatedPlayers;

// ui
@property (nonatomic, strong) UIImage* currencyImage;

// lookup
@property (nonatomic, strong) NSDictionary* currencyImageLookup;

@end

@implementation TBPlayerSeatingViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // currency image lookup
    [self setCurrencyImageLookup:@{@"USD":[UIImage imageNamed:@"b_note_dollar"],
                                   @"EUR":[UIImage imageNamed:@"b_note_euro"],
                                   @"INR":[UIImage imageNamed:@"b_note_rupee"],
                                   @"GBP":[UIImage imageNamed:@"b_note_sterling"],
                                   @"JPY":[UIImage imageNamed:@"b_note_yen_yuan"],
                                   @"CNY":[UIImage imageNamed:@"b_note_yen_yuan"]}];

    // register for KVO
    [[self KVOController] observe:[self session] keyPaths:@[@"seatedPlayers"] options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        // filter and sort
        NSArray* seatedPlayers = [[self session] seatedPlayers];

        // get lists of players
        NSArray* seated = [seatedPlayers filteredArrayUsingPredicate:[NSPredicate predicateWithFormat: @"seat_number != nil"]];
        NSArray* unseated = [seatedPlayers filteredArrayUsingPredicate:[NSPredicate predicateWithFormat: @"seat_number = nil"]];

        // sort descriptors
        NSSortDescriptor* tableNumberSort = [[NSSortDescriptor alloc] initWithKey:@"table_number" ascending:YES];
        NSSortDescriptor* seatNumberSort = [[NSSortDescriptor alloc] initWithKey:@"seat_number" ascending:YES];
        NSSortDescriptor* nameSort = [[NSSortDescriptor alloc] initWithKey:@"player.name" ascending:YES];

        // sorted
        [self setSeatedPlayers:[seated sortedArrayUsingDescriptors:@[tableNumberSort, seatNumberSort]]];
        [self setUnseatedPlayers:[unseated sortedArrayUsingDescriptors:@[nameSort]]];

        // update table view cell
        [[observer tableView] reloadData];
    }];

    [[self KVOController] observe:[self session] keyPath:@"costCurrency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        NSString* costCurrency = [[self session] costCurrency];
        [self setCurrencyImage:[self currencyImageLookup][costCurrency]];
        [[observer tableView] reloadData];
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dealloc {
}

#pragma mark Operations

- (NSString*)displayStringForTableOrSeatNumber:(NSNumber*)number {
    return [@([number integerValue] + 1) stringValue];
}

#pragma mark UITableViewDataSource

- (NSString*)tableView:(UITableView*)tableView titleForHeaderInSection:(NSInteger)section {
    NSString* sectionName;
    switch(section) {
        case 0:
            sectionName = NSLocalizedString(@"Seated", nil);
            break;
        case 1:
            sectionName = NSLocalizedString(@"Unseated", nil);
            break;
        default:
            sectionName = @"";
            break;
    }
    return sectionName;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 2;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    NSArray* players;
    switch(section) {
        case 0:
            players = [[self seatedPlayers] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat: @"seat_number != nil"]];
            break;
        case 1:
            players = [[self unseatedPlayers] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat: @"seat_number = nil"]];
            break;
        default:
            players = nil;
            break;
    }
    return [players count];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell;
    NSDictionary* player;
    switch(indexPath.section) {
        case 0:
            // create a cell
            cell = [tableView dequeueReusableCellWithIdentifier:@"SeatedCell" forIndexPath:indexPath];

            // get player for this row
            player = [self seatedPlayers][[indexPath row]];

            // setup cell
            [(UILabel*)[cell viewWithTag:100] setText:[self displayStringForTableOrSeatNumber:player[@"table_number"]]];
            [(UILabel*)[cell viewWithTag:101] setText:[self displayStringForTableOrSeatNumber:player[@"seat_number"]]];
            [(UILabel*)[cell viewWithTag:102] setText:player[@"player"][@"name"]];

            if([player[@"buyin"] boolValue]) {
                [(UIImageView*)[cell viewWithTag:103] setImage:[self currencyImage]];
            } else {
                [(UIImageView*)[cell viewWithTag:103] setImage:nil];
            }
            break;
        case 1:
            // create a cell
            cell = [tableView dequeueReusableCellWithIdentifier:@"UnseatedCell" forIndexPath:indexPath];

            // get player for this row
            player = [self unseatedPlayers][[indexPath row]];

            // setup cell
            [(UILabel*)[cell viewWithTag:200] setText:player[@"player"][@"name"]];
            break;
        default:
            cell = nil;
            break;
    }
    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    // pop action sheet or something
}

@end
