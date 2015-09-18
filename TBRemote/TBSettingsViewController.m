
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
#import "TBAppDelegate.h"
#import "NSObject+FBKVOController.h"

@interface TBSettingsViewController () <UITableViewDelegate, UITableViewDataSource>

@property (nonatomic, strong) TournamentDaemon* server;
@property (nonatomic, strong) TournamentSession* session;
@property (nonatomic, strong) NSMutableDictionary* configuration;

// number of players to plan for
@property (nonatomic) NSInteger maxPlayers;

// ui references
@property (nonatomic, weak) IBOutlet UIStepper* maxPlayersStepper;

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
    _maxPlayers = 2;

    // load configuration
    NSError* error;
    if([[self class] loadConfig:[self configuration] withError:&error] == NO) {
        // TODO: something with the error
        NSLog(@"%@", error);
    }

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

    // if number of players changes, limit our seating plan
    [[self KVOController] observe:[[self session] state] keyPath:@"players" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        NSUInteger playerCount = [object[@"players"] count];
        [[self maxPlayersStepper] setMaximumValue:(double)playerCount];
    }];

    // if table sizes change, force a replan
    [[self KVOController] observe:[[self session] state] keyPath:@"table_capacity" options:0 block:^(id observer, id object, NSDictionary *change) {
        [self planSeating];
    }];

    // if max players changes, force a replan
    [[self KVOController] observe:self keyPath:@"maxPlayers" options:0 block:^(id observer, id object, NSDictionary *change) {
        [self planSeating];
    }];

    // update rows when keypaths change
    [self bindTableViewRow:0 inSection:0 toObject:[[self session] state] keyPath:@"name"];
    [self bindTableViewRow:1 inSection:0 toObject:[[self session] state] keyPath:@"players"];
    [self bindTableViewRow:2 inSection:0 toObject:[[self session] state] keyPath:@"available_chips"];
    [self bindTableViewRow:3 inSection:0 toObject:[[self session] state] keyPath:@"table_capacity"];
    [self bindTableViewRow:4 inSection:0 toObject:[[self session] state] keyPath:@"buyin_text"];
    [self bindTableViewRow:5 inSection:0 toObject:[[self session] state] keyPath:@"blind_levels"];
    [self bindTableViewRow:6 inSection:0 toObject:[[self session] state] keyPath:@"authorized_clients"];
    [self bindTableViewRow:0 inSection:1 toObject:self keyPath:@"maxPlayers"];

    // Start serving using this device's auth key
    NSString* path = [[self server] startWithAuthCode:[TournamentSession clientIdentifier]];

    // Start the session, connecting locally
    if(![[self session] connectToLocalPath:path]) {
        // TODO: handle error
    }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark UITableView data binding

- (void)bindTableViewRow:(NSInteger)row inSection:(NSInteger)secton toObject:(id)object keyPath:(NSString*)keyPath {
    [[[self tableView] KVOController] observe:object keyPath:keyPath options:0 block:^(id observer, id object, NSDictionary* change) {
        // reload the table view row
        NSIndexPath* theRow = [NSIndexPath indexPathForRow:row inSection:secton];
        [observer reloadRowsAtIndexPaths:@[theRow] withRowAnimation:UITableViewRowAnimationNone];
    }];
}

#pragma mark UITableViewDataSource

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];
    NSDictionary* state = [[self session] state];
    NSString* detail;

    if(indexPath.section == 0) {
        // create a cell
        switch(indexPath.row) {
            case 0:
                detail = state[@"name"];
                [[cell detailTextLabel] setText:detail];
                break;
            case 1:
                detail = [NSString stringWithFormat:@"%ld", [state[@"players"] count]];
                [[cell detailTextLabel] setText:detail];
                break;
            case 2:
                detail = [NSString stringWithFormat:@"%ld", [state[@"available_chips"] count]];
                [[cell detailTextLabel] setText:detail];
                break;
            case 3:
                detail = [NSString stringWithFormat:@"%@", state[@"table_capacity"]];
                [[cell detailTextLabel] setText:detail];
                break;
            case 4:
                detail = state[@"buyin_text"];
                [[cell detailTextLabel] setText:detail];
                break;
            case 5:
                detail = [NSString stringWithFormat:@"%ld", [state[@"blind_levels"] count]];
                [[cell detailTextLabel] setText:detail];
                break;
            case 6:
                detail = [NSString stringWithFormat:@"%ld", [state[@"authorized_clients"] count]];
                [[cell detailTextLabel] setText:detail];
                break;
        }
    } else if(indexPath.section == 1) {
        switch(indexPath.row) {
            case 0:
                detail = [NSString stringWithFormat:@"%ld", [self maxPlayers]];
                [[cell detailTextLabel] setText:detail];

                NSUInteger playerCount = [state[@"players"] count];
                UIStepper* stepper = (UIStepper*)[cell viewWithTag:100];
                [stepper setValue:(double)[self maxPlayers]];
                [stepper setMinimumValue:2.0];
                [stepper setMaximumValue:(double)playerCount];
                break;
        }
    }
    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    if(indexPath.section == 0) {
        // create a cell
        switch(indexPath.row) {
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                break;
            case 6:
                break;
        }
    } else if(indexPath.section == 1) {
        switch(indexPath.row) {
            case 0:
                break;
            case 1:
                // force plan seating
                [self planSeating];
                break;
        }
    }

    // deselect
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
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
