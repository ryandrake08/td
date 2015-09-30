
//
//  TBSettingsViewController.m
//  td
//
//  Created by Ryan Drake on 9/9/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSettingsViewController.h"
#import "TournamentDaemon.h"
#import "TournamentSession.h"
#import "TBSetupTableViewController.h"
#import "TBEditableTableViewCell.h"
#import "TBAppDelegate.h"
#import "NSObject+FBKVOController.h"

@interface TBSettingsViewController () <UITableViewDelegate, UITableViewDataSource>

@property (nonatomic, strong) TournamentDaemon* server;
@property (nonatomic, strong) TournamentSession* session;
@property (nonatomic, strong) NSMutableDictionary* configuration;

// number of players to plan for
@property (nonatomic) NSInteger maxPlayers;

@end

@implementation TBSettingsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get app-wide session
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // server owned by this class
    _server = [[TournamentDaemon alloc] init];

    // for setting up tournament
    _configuration = [[NSMutableDictionary alloc] init];

    // load configuration
    NSError* error;
    if([[self class] loadConfig:[self configuration] withError:&error] == NO) {
        // TODO: something with the error
        NSLog(@"%@", error);
    }

    // default maxPlayers to number of configured players
    [self setMaxPlayers:[[self configuration][@"players"] count]];

    // register for KVO
    [[self KVOController] observe:[[self session] state] keyPaths:@[@"connected", @"authorized"] options:0 block:^(id observer, id object, NSDictionary *change) {
        if([object[@"connected"] boolValue] && [object[@"authorized"] boolValue]) {
            NSLog(@"Connected and authorized locally");

            // on connection, send entire configuration to session, unconditionally, and then replace with whatever the session has
            NSLog(@"Synchronizing session unconditionally");
            [[self session] configure:[self configuration] withBlock:^(id json) {
                if(![json isEqual:[self configuration]]) {
                    NSLog(@"Document differs from session");
                    [[self configuration] setDictionary:json];

                    // reload the table view
                    [[self tableView] reloadData];
                }
            }];
        }
    }];

    // whenever tournament name changes, do some stuff
    [[self KVOController] observe:[[self session] state] keyPath:@"name" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        // re-publish on Bonjour
        [[self server] publishWithName:object[@"name"]];
    }];

    // if table sizes change, force a replan
    [[self KVOController] observe:[[self session] state] keyPath:@"table_capacity" options:0 block:^(id observer, id object, NSDictionary *change) {
        [self planSeating];
    }];

    // if max players changes, force a replan
    [[self KVOController] observe:self keyPath:@"maxPlayers" options:0 block:^(id observer, id object, NSDictionary *change) {
        [self planSeating];
    }];

    // update row when maxPlayers changes
    [[[self tableView] KVOController] observe:self keyPath:@"maxPlayers" options:0 block:^(id observer, id object, NSDictionary* change) {
        // reload the table view row
        NSIndexPath* theRow = [NSIndexPath indexPathForRow:0 inSection:1];
        [observer reloadRowsAtIndexPaths:@[theRow] withRowAnimation:UITableViewRowAnimationNone];
    }];

    // Start serving using this device's auth key
    NSString* path = [[self server] startWithAuthCode:[TournamentSession clientIdentifier]];

    // Start the session, connecting locally
    if(![[self session] connectToLocalPath:path]) {
        // TODO: handle error
    }
}

- (void)viewWillAppear:(BOOL)animated {
    NSIndexPath* selectedRowIndexPath = [[self tableView] indexPathForSelectedRow];
    [super viewWillAppear:animated];
    if (selectedRowIndexPath) {
        [[self tableView] reloadRowsAtIndexPaths:@[selectedRowIndexPath] withRowAnimation:UITableViewRowAnimationNone];
    }
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
                detail = [NSString stringWithFormat:@"%ld", [[self configuration][@"players"] count]];
                [[cell detailTextLabel] setText:detail];
                break;
            case 2:
                detail = [NSString stringWithFormat:@"%ld", [[self configuration][@"available_chips"] count]];
                [[cell detailTextLabel] setText:detail];
                break;
            case 3:
                [(TBPickableTextTableViewCell*)cell setAllowedValues:@[@2, @3, @4, @5, @6, @7, @8, @9, @10, @11, @12] withTitles:nil];
                [(TBEditableTableViewCell*)cell setObject:[self configuration]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"table_capacity"];
                break;
            case 4:
                detail = [self configuration][@"buyin_text"];
                [[cell detailTextLabel] setText:detail];
                break;
            case 5:
                detail = [NSString stringWithFormat:@"%ld", [[self configuration][@"blind_levels"] count]-1];
                [[cell detailTextLabel] setText:detail];
                break;
            case 6:
                detail = [NSString stringWithFormat:@"%ld", [[self configuration][@"authorized_clients"] count]];
                [[cell detailTextLabel] setText:detail];
                break;
        }
    } else if(indexPath.section == 1) {
        switch(indexPath.row) {
            case 0:
                detail = [NSString stringWithFormat:@"%ld", [self maxPlayers]];
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
            [self planSeating];
        }

        // deselect
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
    }
}

#pragma mark Navigation

- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    TBSetupTableViewController* newController = [segue destinationViewController];
    [newController setConfiguration:[self configuration]];
}

#pragma mark Operations

- (void)planSeating {
    NSLog(@"Planning seating for %ld players", [self maxPlayers]);
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

#pragma mark File handling

#define kHardcodedDocumentName @"mobile.tournbuddy"

+ (BOOL)saveConfig:(NSDictionary*)config withError:(NSError**)error {
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSURL* docDirectoryUrl = [fileManager URLForDirectory:NSDocumentDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:error];
    if(docDirectoryUrl != nil) {
        NSURL* docUrl = [docDirectoryUrl URLByAppendingPathComponent:kHardcodedDocumentName];

        // serialize data
        NSData* data = [NSJSONSerialization dataWithJSONObject:config options:0 error:error];
        if(data != nil) {
            // write data to disk
            return [data writeToURL:docUrl options:0 error:error];
        }
    }
    return *error == nil;
}

+ (BOOL)loadConfig:(NSMutableDictionary*)config withError:(NSError**)error {
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSURL* docDirectoryUrl = [fileManager URLForDirectory:NSDocumentDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:error];
    if(docDirectoryUrl != nil) {
        NSURL* docUrl = [docDirectoryUrl URLByAppendingPathComponent:kHardcodedDocumentName];

        // check existance of file
        if(![fileManager fileExistsAtPath:[docUrl path]]) {
            // file does not exist, copy default
            NSString* defaultDocument = [[NSBundle mainBundle] pathForResource:@"defaults" ofType:@"json"];
            if(![fileManager copyItemAtPath:defaultDocument toPath:[docUrl path] error:error]) {
                return NO;
            }
        }

        // read data from disk
        NSData* data = [NSData dataWithContentsOfURL:docUrl options:0 error:error];
        if(data != nil) {
            [config setDictionary:[NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:error]];
        }
    }
    return *error == nil;
}

@end
