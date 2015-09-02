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
    [[self KVOController] observe:[[self session] state] keyPaths:@[@"seated_players"] options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        // filter and sort
        NSArray* seatedPlayers = object[@"seated_players"];

        // get lists of players
        NSArray* seated = [seatedPlayers filteredArrayUsingPredicate:[NSPredicate predicateWithFormat: @"seat_number != nil"]];
        NSArray* unseated = [seatedPlayers filteredArrayUsingPredicate:[NSPredicate predicateWithFormat: @"seat_number = nil"]];

        // sort descriptors
        NSSortDescriptor* tableNumberSort = [[NSSortDescriptor alloc] initWithKey:@"table_number" ascending:YES];
        NSSortDescriptor* seatNumberSort = [[NSSortDescriptor alloc] initWithKey:@"seat_number" ascending:YES];
        NSSortDescriptor* nameSort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES];

        // sorted
        [self setSeatedPlayers:[seated sortedArrayUsingDescriptors:@[tableNumberSort, seatNumberSort]]];
        [self setUnseatedPlayers:[unseated sortedArrayUsingDescriptors:@[nameSort]]];

        // update table view cell
        [[observer tableView] reloadData];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"cost_currency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        NSString* costCurrency = object[@"cost_currency"];
        [self setCurrencyImage:[self currencyImageLookup][costCurrency]];
        [[observer tableView] reloadData];
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
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
        [(UILabel*)[cell viewWithTag:102] setText:player[@"name"]];

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
        [(UILabel*)[cell viewWithTag:200] setText:player[@"name"]];
    }
    return cell;
}

#pragma mark UITableViewDelegate

#define kCommandSeatPlayer -1
#define kCommandUnseatPlayer -2
#define kCommandBustPlayer -3

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    if([[self session] isConnected] && [[self session] isAuthorized]) {
        NSDictionary* player;
        UIActionSheet* actionSheet = [[UIActionSheet alloc] initWithTitle:nil
                                                                 delegate:self
                                                        cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
                                                   destructiveButtonTitle:nil
                                                        otherButtonTitles:nil];

        NSMutableArray* commands = [[NSMutableArray alloc] initWithObjects:@0, nil];

        if(indexPath.section == 0) {
            NSNumber* currentBlindLevel = [[self session] state][@"current_blind_level"];

            // get player for this row
            player = [self seatedPlayers][[indexPath row]];

            // add unseat button
            [actionSheet addButtonWithTitle:NSLocalizedString(@"Unseat Player", nil)];
            [commands addObject:@(kCommandUnseatPlayer)];

            if([currentBlindLevel unsignedIntegerValue] > 0) {
                // add unseat button
                [actionSheet addButtonWithTitle:NSLocalizedString(@"Bust Player", nil)];
                [commands addObject:@(kCommandBustPlayer)];
            }

            // add buttons for each eligible funding source
            [[[self session] state][@"funding_sources"] enumerateObjectsUsingBlock:^(id source, NSUInteger idx, BOOL* stop) {
                NSNumber* last = source[@"forbid_after_blind_level"];
                if(last == nil || !([last compare:currentBlindLevel] == NSOrderedAscending)) {
                    [actionSheet addButtonWithTitle:source[@"name"]];
                    [commands addObject:@(idx)];
                }
            }];
        } else if(indexPath.section == 1) {
            // get player for this row
            player = [self unseatedPlayers][[indexPath row]];

            // set buttons
            [actionSheet addButtonWithTitle:NSLocalizedString(@"Seat Player", nil)];
            [commands addObject:@(kCommandSeatPlayer)];
        }

        // set context
        NSDictionary* context = @{@"player_id":player[@"player_id"],@"commands":commands};
        [actionSheet setAssociatedObject:context];

        // pop actionsheet
        [actionSheet showInView:[self view]];
    }

    // deselect either way
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

#pragma mark UIActionSheetDelegate

- (void)actionSheet:(UIActionSheet*)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
    if(buttonIndex != [actionSheet cancelButtonIndex]) {
        NSDictionary* context = [actionSheet associatedObject];
        NSString* playerId = context[@"player_id"];
        NSArray* commands = context[@"commands"];

        switch([commands[buttonIndex] integerValue]) {
            case kCommandSeatPlayer:
                [[self session] seatPlayer:playerId withBlock:nil];
                break;

            case kCommandUnseatPlayer:
                [[self session] unseatPlayer:playerId withBlock:nil];
                break;

            case kCommandBustPlayer:
                [[self session] bustPlayer:playerId withBlock:nil];
                break;

            default:
                [[self session] fundPlayer:playerId withFunding:commands[buttonIndex]];
                break;
        }
    }
}

@end
