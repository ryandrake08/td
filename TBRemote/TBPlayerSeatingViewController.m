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

#import "NSObject+AssociatedObject.h"
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
    if(section == 0) {
        sectionName = NSLocalizedString(@"Seated", nil);
    } else if(section == 1) {
        sectionName = NSLocalizedString(@"Unseated", nil);
    }
    return sectionName;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 2;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    NSArray* players;
    if(section == 0) {
        players = [[self seatedPlayers] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat: @"seat_number != nil"]];
    } else if(section == 1) {
        players = [[self unseatedPlayers] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat: @"seat_number = nil"]];
    }
    return [players count];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell;
    NSDictionary* player;
    if(indexPath.section == 0) {
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
    } else if(indexPath.section == 1) {
        // create a cell
        cell = [tableView dequeueReusableCellWithIdentifier:@"UnseatedCell" forIndexPath:indexPath];

        // get player for this row
        player = [self unseatedPlayers][[indexPath row]];

        // setup cell
        [(UILabel*)[cell viewWithTag:200] setText:player[@"player"][@"name"]];
    }
    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    if([[self session] isConnected] && [[self session] isAuthorized]) {
        NSDictionary* player;
        UIActionSheet* actionSheet = [[UIActionSheet alloc] initWithTitle:nil
                                                                 delegate:self
                                                        cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
                                                   destructiveButtonTitle:nil
                                                        otherButtonTitles:nil];


        if(indexPath.section == 0) {
            // get player for this row
            player = [self seatedPlayers][[indexPath row]];

            // set buttons
            [actionSheet addButtonWithTitle:NSLocalizedString(@"Unseat Player", nil)];
            [actionSheet addButtonWithTitle:NSLocalizedString(@"Bust Player", nil)];

            [[[self session] fundingSources] enumerateObjectsUsingBlock:^(id source, NSUInteger idx, BOOL* stop) {
                [actionSheet addButtonWithTitle:source[@"name"]];
            }];
        } else if(indexPath.section == 1) {
            // get player for this row
            player = [self unseatedPlayers][[indexPath row]];

            // set buttons
            [actionSheet addButtonWithTitle:NSLocalizedString(@"Seat Player", nil)];
        }

        // pop actionsheet
        [actionSheet setAssociatedObject:player];
        [actionSheet showInView:[self view]];
    }

    // deselect either way
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

#pragma mark UIActionSheetDelegate

- (void)actionSheet:(UIActionSheet*)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
    NSDictionary* player = [actionSheet associatedObject];
    NSString* playerId = player[@"player"][@"player_id"];

    if(player[@"seat_number"] == nil) {
        NSInteger opsIdx = buttonIndex - 1;
        if(opsIdx == 0) {
            [[self session] seatPlayer:playerId withBlock:nil];
        }
    } else {
        NSInteger opsIdx = buttonIndex - 1;
        NSInteger fundingIdx = buttonIndex - 3;
        if(opsIdx == 0) {
            [[self session] unseatPlayer:playerId withBlock:nil];
        } else if(opsIdx == 1) {
            [[self session] bustPlayer:playerId withBlock:nil];
        } else {
            [[self session] fundPlayer:playerId withFunding:@(fundingIdx)];
        }
    }
}

@end
