
//
//  TBSettingsViewController.m
//  td
//
//  Created by Ryan Drake on 9/9/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSettingsViewController.h"
#import "NSObject+FBKVOController.h"
#import "TBAppDelegate.h"
#import "TBEditableTableViewCell.h"
#import "TBNotifications.h"
#import "TBSetupTableViewController.h"
#import "TournamentDaemon.h"
#import "TournamentSession.h"
#import "UIAlertView+Blocks.h"
#import "UIResponder+PresentingErrors.h"

@interface TBSettingsViewController () <UITableViewDelegate, UITableViewDataSource>

@property (nonatomic, strong) TournamentDaemon* server;
@property (nonatomic, strong) TournamentSession* session;
@property (nonatomic, strong) NSMutableDictionary* configuration;

// number of players to plan for
@property (nonatomic) NSUInteger maxPlayers;

// current document url
@property (nonatomic, copy) NSURL* documentURL;

@end

@implementation TBSettingsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get app-wide session
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // server owned by this class
    _server = [[TournamentDaemon alloc] init];

    // Start serving using this device's auth key
    TournamentService* service = [[self server] startWithAuthCode:[TournamentSession clientIdentifier]];

    // Start the session, connecting locally
    NSError* error;
    if(![[self session] connectToTournamentService:service error:&error]) {
        [self presentError:error];
    }

    // register for KVO
    [[self KVOController] observe:self keyPaths:@[@"session.state.connected", @"session.state.authorized"] options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        // send config to session
        if([[[self session] state][@"connected"] boolValue] && [[[self session] state][@"authorized"] boolValue]) {
            [[self session] selectiveConfigure:[self configuration] withBlock:nil];
        }
    }];

    // whenever tournament name changes, do some stuff
    [[self KVOController] observe:self keyPath:@"configuration.name" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        // re-publish on Bonjour
        [[self server] publishWithName:[self configuration][@"name"]];

        // reload table
        [[self tableView] reloadData];
    }];

    // whenever other configuration changes, update table row
    [[self KVOController] observe:self keyPaths:@[@"configuration.players", @"configuration.available_chips", @"configuration.funding_sources", @"configuration.blind_levels", @"configuration.authorized_clients"] options:0 block:^(id observer, id object, NSDictionary *change) {
        // reload table
        [[self tableView] reloadData];
    }];

    // if table sizes or max players change, force a replan
    [[self KVOController] observe:self keyPaths:@[@"configuration.table_capacity", @"maxPlayers"] options:0 block:^(id observer, id object, NSDictionary *change) {
        // plan seating
        [self planSeating];

        // reload table
        [[self tableView] reloadData];
    }];

    // send to session and save every time configuration changes
    [[NSNotificationCenter defaultCenter] addObserverForName:kConfigurationUpdatedNotification object:nil queue:nil usingBlock:^(NSNotification* note) {
        // send to session
        if([[[self session] state][@"connected"] boolValue] && [[[self session] state][@"authorized"] boolValue]) {
            [[self session] selectiveConfigure:[self configuration] withBlock:nil];
        }

        // save
        [self saveConfig];
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark UITableViewDataSource

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];
    NSString* detail;

    if(indexPath.section == 0) {
        // create a cell
        switch(indexPath.row) {
            case 0:
            {
                [(TBEditableTableViewCell*)cell setObject:[self configuration]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"name"];
                break;
            }
            case 1:
                detail = [NSString stringWithFormat:@"%lu", (unsigned long)[[self configuration][@"players"] count]];
                [(UILabel*)[cell viewWithTag:100] setText:detail];
                break;
            case 2:
                detail = [NSString stringWithFormat:@"%lu", (unsigned long)[[self configuration][@"available_chips"] count]];
                [(UILabel*)[cell viewWithTag:100] setText:detail];
                break;
            case 3:
                detail = [NSString stringWithFormat:@"%lu", (unsigned long)[[self configuration][@"funding_sources"] count]];
                [(UILabel*)[cell viewWithTag:100] setText:detail];
                break;
            case 4:
                detail = [NSString stringWithFormat:@"%lu", (unsigned long)[[self configuration][@"blind_levels"] count]-1];
                [(UILabel*)[cell viewWithTag:100] setText:detail];
                break;
            case 5:
                detail = [NSString stringWithFormat:@"%lu", (unsigned long)[[self configuration][@"authorized_clients"] count]-1];
                [(UILabel*)[cell viewWithTag:100] setText:detail];
                break;
            case 6:
                [(TBPickableTextTableViewCell*)cell setAllowedValues:@[@2, @3, @4, @5, @6, @7, @8, @9, @10, @11, @12] withTitles:nil];
                [(TBEditableTableViewCell*)cell setObject:[self configuration]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"table_capacity"];
                break;
        }
    } else if(indexPath.section == 1) {
        switch(indexPath.row) {
            case 0:
                detail = [NSString stringWithFormat:@"%lu", (unsigned long)[self maxPlayers]];
                [(UILabel*)[cell viewWithTag:101] setText:detail];

                UIStepper* stepper = (UIStepper*)[cell viewWithTag:100];
                [stepper setValue:(double)[self maxPlayers]];
                NSUInteger numPlayers = [[[self session] state][@"players"] count];
                if(numPlayers > 1) {
                    [stepper setMaximumValue:numPlayers * 2.0];
                }
                break;
        }
    }
    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    if([indexPath section] == 1) {
        if([indexPath row] == 1) {
            // force plan seating
            [self planSeatingTapped:self];
        } else if([indexPath row] == 2) {
            // quick setup
            [self quickStartTapped:self];
        }

        // deselect
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
    } else if([indexPath section] == 2) {
        if([indexPath row] == 0) {
            // open
        } else if([indexPath row] == 1) {
            // save
        }
    }
}

#pragma mark Navigation

- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    TBSetupTableViewController* newController = [segue destinationViewController];
    [newController setConfiguration:[self configuration]];
}

#pragma mark Operations

- (void)planSeating {
    NSLog(@"Planning seating for %lu players", (unsigned long)[self maxPlayers]);
    if([self maxPlayers] > 1) {
        [[self session] planSeatingFor:@([self maxPlayers])];
    }
}

#pragma mark Actions

- (IBAction)stepperDidChange:(id)sender {
    if([sender isKindOfClass:[UIStepper class]]) {
        UIStepper* stepper = (UIStepper*)sender;
        NSInteger value = (NSInteger)[stepper value];
        [self setMaxPlayers:value];
    }
}

- (void)planSeatingTapped:(id)sender {
    if([self maxPlayers] > 1) {
        if([[[self session] state][@"seats"] count] > 0 || [[[self session] state][@"buyins"] count] > 0) {
            // display a different message if the game is running
            BOOL playing = [[[self session] state][@"current_blind_level"] unsignedIntegerValue] != 0;
            NSString* message;
            if(playing) {
                message = NSLocalizedString(@"This will end the current tournament immediately, then clear any existing seats and buy-ins.", nil);
            } else {
                message = NSLocalizedString(@"This will clear any existing seats and buy-ins.", nil);
            }

            // alert because this is a very destructive action
            [UIAlertView showWithTitle:NSLocalizedString(@"Plan Seating", nil)
                               message:message
                     cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
                     otherButtonTitles:@[NSLocalizedString(@"Plan", nil)]
                              tapBlock:^(UIAlertView* alertView, NSInteger buttonIndex) {
                                  if(buttonIndex != [alertView cancelButtonIndex]) {
                                      NSLog(@"Planning seating for %lu players", (unsigned long)[self maxPlayers]);
                                      [[self session] planSeatingFor:@([self maxPlayers])];

                                      // switch to clock screen automatically
                                      [[[self navigationController] tabBarController] setSelectedIndex:1];
                                  }
                              }];
        } else {
            // no warning
            NSLog(@"Planning seating for %lu players", (unsigned long)[self maxPlayers]);
            [[self session] planSeatingFor:@([self maxPlayers])];

            // switch to clock screen automatically
            [[[self navigationController] tabBarController] setSelectedIndex:1];
        }
    }
}

- (IBAction)quickStartTapped:(id)sender {
    if([[[self session] state][@"seats"] count] > 0 || [[[self session] state][@"buyins"] count] > 0) {
        // display a different message if the game is running
        BOOL playing = [[[self session] state][@"current_blind_level"] unsignedIntegerValue] != 0;
        NSString* message;
        if(playing) {
            message = NSLocalizedString(@"Quick Start will end the current tournament immediately, then re-seat and buy in all players.", nil);
        } else {
            message = NSLocalizedString(@"Quick Start will clear any existing seats and buy-ins, then re-seat and buy in all players.", nil);
        }

        // alert because this is a very destructive action
        [UIAlertView showWithTitle:NSLocalizedString(@"Quick Setup", nil)
                           message:message
                 cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
                 otherButtonTitles:@[NSLocalizedString(@"Setup", nil)]
                          tapBlock:^(UIAlertView* alertView, NSInteger buttonIndex) {
                              if(buttonIndex != [alertView cancelButtonIndex]) {
                                  [[self session] quickSetupWithBlock:nil];

                                  // switch to clock screen automatically
                                  [[[self navigationController] tabBarController] setSelectedIndex:2];
                              }
                          }];
    } else {
        // no warning
        [[self session] quickSetupWithBlock:nil];

        // switch to clock screen automatically
        [[[self navigationController] tabBarController] setSelectedIndex:2];
    }
}

#pragma mark File handling

- (BOOL)saveConfig {
    // serialize data
    NSError* error;
    NSData* data = [NSJSONSerialization dataWithJSONObject:[self configuration] options:0 error:&error];
    if(data == nil) {
        [self presentError:error];
        return NO;
    }

    // write data to disk
    if(![data writeToURL:[self documentURL] options:0 error:&error]) {
        [self presentError:error];
        return NO;
    }

    return YES;
}

- (BOOL)loadDocumentFromContentsOfURL:(NSURL*)docUrl {
    NSError* error;

    // read data from disk
    NSData* data = [NSData dataWithContentsOfURL:docUrl options:0 error:&error];
    if(data == nil) {
        [self presentError:error];
        return NO;
    }

    // deserialize data
    id dict = [NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:&error];
    if(dict == nil) {
        [self presentError:error];
        return NO;
    }

    // replace current configuration
    [self setConfiguration:dict];

    // store url for later serialization
    [self setDocumentURL:docUrl];

    // default maxPlayers to number of configured players
    [self setMaxPlayers:[[self configuration][@"players"] count]];

    // unconditionally send to session
    if([[[self session] state][@"connected"] boolValue] && [[[self session] state][@"authorized"] boolValue]) {
        [[self session] configure:dict withBlock:nil];
    }

    return YES;
}

- (BOOL)loadDocumentFromContentsOfFile:(NSString*)fileName {
    NSError* error;
    NSFileManager* fileManager = [NSFileManager defaultManager];

    NSURL* docDirectoryUrl = [fileManager URLForDirectory:NSDocumentDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:&error];
    if(docDirectoryUrl == nil) {
        [self presentError:error];
        return NO;
    }

    // compute document url
    NSURL* docUrl = [docDirectoryUrl URLByAppendingPathComponent:fileName];

    // check existance of file
    if(![fileManager fileExistsAtPath:[docUrl path]]) {
        // file does not exist, copy default
        NSString* defaultDocument = [[NSBundle mainBundle] pathForResource:@"defaults" ofType:@"json"];
        if(![fileManager copyItemAtPath:defaultDocument toPath:[docUrl path] error:&error]) {
            [self presentError:error];
            return NO;
        }
    }

    // load as url
    return [self loadDocumentFromContentsOfURL:docUrl];
}

@end
