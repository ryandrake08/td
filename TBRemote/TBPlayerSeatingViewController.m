//
//  TBPlayerSeatingViewController.m
//  td
//
//  Created by Ryan Drake on 8/28/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBPlayerSeatingViewController.h"
#import "NSObject+FBKVOController.h"
#import "TBAppDelegate.h"
#import "TBCurrencyImageTransformer.h"
#import "TBNotifications.h"
#import "TournamentSession.h"

@interface TBPlayerSeatingViewController () <UITableViewDelegate, UITableViewDataSource>

@property (nonatomic, strong) TournamentSession* session;

// tableview content
@property (nonatomic, strong) NSArray* seatedPlayers;
@property (nonatomic, strong) NSArray* unseatedPlayers;

// ui
@property (nonatomic, strong) UIImage* currencyImage;

// store funding source for buy-in
@property (nonatomic, strong) NSDictionary* buyinFundingSource;

@end

@implementation TBPlayerSeatingViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // default currency image
    [self setCurrencyImage:[UIImage imageNamed:@"b_note_dollar"]];

    // register for KVO
    [[self KVOController] observe:self keyPath:@"session.state.seated_players" options:NSKeyValueObservingOptionInitial block:^(id observer, TBPlayerSeatingViewController* object, NSDictionary *change) {
        // filter and sort
        NSArray* seatedPlayers = [[object session] state][@"seated_players"];

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

        // update table view cells
        [[observer tableView] reloadData];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.funding_sources" options:NSKeyValueObservingOptionInitial block:^(id observer, TBPlayerSeatingViewController* object, NSDictionary *change) {
        // find first buy-in
        [[[object session] state][@"funding_sources"] enumerateObjectsUsingBlock:^(id source, NSUInteger idx, BOOL* stop) {
            if([source[@"type"] isEqual:kFundingTypeBuyin]) {
                [self setBuyinFundingSource:source];
                *stop = YES;
            }
        }];

        // update currency image
        NSString* buyinCurrency = [self buyinFundingSource][@"cost_currency"];
        if(buyinCurrency != nil) {
            TBCurrencyImageTransformer* imageTransformer = [[TBCurrencyImageTransformer alloc] init];
            UIImage* image = [imageTransformer transformedValue:buyinCurrency];
            if(image) {
                [self setCurrencyImage:image];
            }
        }

        // update table view cells
        [[observer tableView] reloadData];
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark Actions

- (IBAction)rebalanceTapped:(id)sender {
    [[self session] rebalanceSeatingWithBlock:^(NSArray* movements) {
        if([movements count] > 0) {
            [[NSNotificationCenter defaultCenter] postNotificationName:kMovementsUpdatedNotification object:movements];
        }
    }];
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
    NSDictionary* player;
    if(indexPath.section == 0) {
        // create a cell
        UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SeatedCell" forIndexPath:indexPath];

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
        return cell;
    } else if(indexPath.section == 1) {
        // create a cell
        UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"UnseatedCell" forIndexPath:indexPath];

        // get player for this row
        player = [self unseatedPlayers][[indexPath row]];

        // setup cell
        [(UILabel*)[cell viewWithTag:200] setText:player[@"name"]];
        return cell;
    } else {
        NSLog(@"TBPlayerSeatingViewController tableView:cellForRowAtIndexPath: invalid section");
        abort();
    }
}

#pragma mark UITableViewDelegate

#define kCommandSeatPlayer -1
#define kCommandUnseatPlayer -2
#define kCommandBustPlayer -3

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    if([[[self session] state][@"connected"] boolValue] && [[[self session] state][@"authorized"] boolValue]) {
        NSDictionary* player;

        UIAlertController* actionSheet = [UIAlertController alertControllerWithTitle:nil message:nil preferredStyle:UIAlertControllerStyleActionSheet];
        [actionSheet addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"Cancel", nil) style:UIAlertActionStyleCancel handler:nil]];

        // action sheet buttons depend on current blind level
        NSNumber* currentBlindLevel = [[self session] state][@"current_blind_level"];

        // get list of players who have already bought in
        NSArray* uniqueEntries = [[self session] state][@"unique_entries"];

        if(indexPath.section == 0) {
            // get player for this row
            player = [self seatedPlayers][[indexPath row]];
            NSString* playerId = player[@"player_id"];

            // set title
            [actionSheet setTitle:player[@"name"]];

            // BUSINESS LOGIC AROUND WHICH FUNDING SOURCES ARE ALLOWED WHEN

            // add buttons for each eligible funding source
            [[[self session] state][@"funding_sources"] enumerateObjectsUsingBlock:^(id source, NSUInteger idx, BOOL* stop) {
                NSNumber* last = source[@"forbid_after_blind_level"];
                if(last == nil || !([last compare:currentBlindLevel] == NSOrderedAscending)) {
                    if([source[@"type"] isEqual:kFundingTypeBuyin]) {
                        // buyins can happen at any time before forbid_after_blind_level, for any non-playing player
                        if(![player[@"buyin"] boolValue]) {
                            [actionSheet addAction:[UIAlertAction actionWithTitle:source[@"name"] style:UIAlertActionStyleDefault handler:^(UIAlertAction* action) {
                                [[self session] fundPlayer:playerId withFunding:@(idx)];
                            }]];
                        }
                    } else if([source[@"type"] isEqual:kFundingTypeRebuy]) {
                        // rebuys can happen after round 0, before forbid_after_blind_level, for any player that has bought in at least once
                        if([currentBlindLevel unsignedIntegerValue] > 0 && [uniqueEntries containsObject:player[@"player_id"]]) {
                            [actionSheet addAction:[UIAlertAction actionWithTitle:source[@"name"] style:UIAlertActionStyleDefault handler:^(UIAlertAction* action) {
                                [[self session] fundPlayer:playerId withFunding:@(idx)];
                            }]];
                        }
                    } else {
                        // addons can happen at any time before forbid_after_blind_level, for any playing player
                        if([player[@"buyin"] boolValue]) {
                            [actionSheet addAction:[UIAlertAction actionWithTitle:source[@"name"] style:UIAlertActionStyleDefault handler:^(UIAlertAction* action) {
                                [[self session] fundPlayer:playerId withFunding:@(idx)];
                            }]];
                        }
                    }
                }
            }];

            // BUSINESS LOGIC AROUND BUSTING AND UN-SEATING

            // set up bust function. if game is running and all selected player is bought in, then enable the bust item
            if([currentBlindLevel unsignedIntegerValue] > 0 && [player[@"buyin"] boolValue]) {
                [actionSheet addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"Bust Player", nil) style:UIAlertActionStyleDefault handler:^(UIAlertAction* action) {
                    [[self session] bustPlayer:playerId withBlock:^(NSArray* movements) {
                        if([movements count] > 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:kMovementsUpdatedNotification object:movements];
                        }
                    }];
                }]];
            }

            // add unseat function. if selected player is not bought in, then enable the unseat item
            if(![player[@"buyin"] boolValue]) {
                [actionSheet addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"Unseat Player", nil) style:UIAlertActionStyleDefault handler:^(UIAlertAction* action) {
                    [[self session] unseatPlayer:playerId withBlock:nil];
                }]];
            }
        } else if(indexPath.section == 1) {
            // get player for this row
            player = [self unseatedPlayers][[indexPath row]];
            NSString* playerId = player[@"player_id"];

            // set title
            [actionSheet setTitle:player[@"name"]];

            // set buttons
            [actionSheet addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"Seat Player", nil) style:UIAlertActionStyleDefault handler:^(UIAlertAction* action) {
                [[self session] seatPlayer:playerId withBlock:nil];
            }]];

            // add buttons for any eligible (buyin only) funding sources
            [[[self session] state][@"funding_sources"] enumerateObjectsUsingBlock:^(id source, NSUInteger idx, BOOL* stop) {
                NSNumber* last = source[@"forbid_after_blind_level"];
                if(last == nil || !([last compare:currentBlindLevel] == NSOrderedAscending)) {
                    if([source[@"type"] isEqual:kFundingTypeBuyin]) {
                        // buyins can happen at any time before forbid_after_blind_level, for any non-playing player
                        if(![player[@"buyin"] boolValue]) {
                            NSString* titleString = [NSLocalizedString(@"Seat Player + ", nil) stringByAppendingString:source[@"name"]];
                            [actionSheet addAction:[UIAlertAction actionWithTitle:titleString style:UIAlertActionStyleDefault handler:^(UIAlertAction* action) {
                                [[self session] seatPlayer:playerId withBlock:nil];
                                [[self session] fundPlayer:playerId withFunding:@(idx)];
                            }]];
                        }
                    }
                }
            }];
        } else {
            return;
        }

        // pop actionsheet
        [self presentViewController:actionSheet animated:YES completion:nil];
    }

    // deselect either way
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
