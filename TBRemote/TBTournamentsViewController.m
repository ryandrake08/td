//
//  TBTournamentsViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTournamentsViewController.h"
#import "TBAppDelegate.h"
#import "TournamentBrowser.h"
#import "TournamentService.h"
#import "TournamentSession.h"
#import "UIResponder+PresentingErrors.h"

#import "NSObject+FBKVOController.h"

@interface TBTournamentsViewController () <UITableViewDelegate,
                                           UITableViewDataSource,
                                           TournamentBrowserDelegate>

@property (nonatomic, strong) TournamentSession* session;
@property (nonatomic, strong) IBOutlet TournamentBrowser* tournamentBrowser;
@property (nonatomic, strong) TournamentService* currentTournamentService;
@property (nonatomic, copy) NSArray* tournamentServices;
@end

@implementation TBTournamentsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // initialize tournament browser
    [[self tournamentBrowser] search];

    // register for KVO
    [[self KVOController] observe:self keyPaths:@[@"session.connected", @"session.authorized"] options:0 block:^(id observer, TBTournamentsViewController* object, NSDictionary *change) {
        // update table view cell
        [observer reloadTableRowForService:[self currentTournamentService]];
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dealloc {
    // stop service browser
    [self setTournamentBrowser:nil];
}


#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return (NSInteger)[[self tournamentServices] count];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"ServiceCell" forIndexPath:indexPath];

    NSUInteger urow = (NSUInteger)[indexPath row];
    TournamentService* cellService = [self tournamentServices][urow];
    BOOL connected = [[self session] connected];
    BOOL authorized = [[self session] authorized];

    // always set name
    [[cell textLabel] setText:[cellService name]];

    // set checkmark and accessory text if connected
    if(cellService == [self currentTournamentService] && connected) {
        if(authorized) {
            [[cell detailTextLabel] setText:NSLocalizedString(@"Admin", nil)];
        } else {
            [[cell detailTextLabel] setText:NSLocalizedString(@"Connected", nil)];
        }
        [cell setAccessoryType:UITableViewCellAccessoryCheckmark];
    } else {
        [[cell detailTextLabel] setText:@""];
        [cell setAccessoryType:UITableViewCellAccessoryNone];
    }

    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    NSUInteger urow = (NSUInteger)[indexPath row];
    TournamentService* cellService = [self tournamentServices][urow];
    BOOL connected = [[self session] connected];
    BOOL authorized = [[self session] authorized];

    if(cellService == [self currentTournamentService] && connected) {
        // pop actionsheet
        UIAlertController* actionSheet = [UIAlertController alertControllerWithTitle:[cellService name] message:nil preferredStyle:UIAlertControllerStyleActionSheet];
        [actionSheet addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"Cancel", nil) style:UIAlertActionStyleCancel handler:nil]];
        [actionSheet addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"Leave Game", nil) style:UIAlertActionStyleDestructive handler:^(UIAlertAction* action) {
            // disconnect
            [[self session] disconnect];
            [self setCurrentTournamentService:nil];
        }]];
        if(!authorized) {
            [actionSheet addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"Administer Game", nil) style:UIAlertActionStyleDefault handler:^(UIAlertAction* actionSheetAction) {
                NSString* message = [NSString stringWithFormat:NSLocalizedString(@"The tournament director needs to authorize this code: %@", nil), [TournamentSession clientIdentifier]];

                // present alert
                UIAlertController* alert = [UIAlertController alertControllerWithTitle:NSLocalizedString(@"Administer Game", nil) message:message preferredStyle:UIAlertControllerStyleAlert];
                [alert addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"Cancel", nil) style:UIAlertActionStyleCancel handler:nil]];
                [alert addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"Administer", nil) style:UIAlertActionStyleDefault handler:^(UIAlertAction* alertAction) {
                    // check authorization
                    [[self session] checkAuthorizedWithBlock:^(BOOL nowAuthorized) {
                        if(nowAuthorized) {
                            // switch to seating screen automatically
                            [[[self navigationController] tabBarController] setSelectedIndex:1];
                        }
                    }];
                }]];
                [self presentViewController:alert animated:YES completion:nil];
            }]];
        }

        // provide popover information for iPad
        UIPopoverPresentationController* popPresenter = [actionSheet popoverPresentationController];
        UIView* sourceView = [tableView cellForRowAtIndexPath:indexPath];
        [popPresenter setSourceView:sourceView];
        [popPresenter setSourceRect:[sourceView bounds]];

        [self presentViewController:actionSheet animated:YES completion:nil];
    } else {
        // connect
        NSError* error;
        if([[self session] connectToTournamentService:cellService error:&error]) {
            // store as current service
            [self setCurrentTournamentService:cellService];

            // switch to clock screen automatically
            [[[self navigationController] tabBarController] setSelectedIndex:2];

        } else {
            [self presentError:error];
        }
    }

    // deselect either way
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)reloadTableRowForService:(TournamentService*)service {
    NSUInteger i = [[self tournamentServices] indexOfObject:service];
    if(i != NSNotFound) {
        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:(NSInteger)i inSection:0];
        [[self tableView] reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

#pragma mark TournamentBroswerDelegate

- (void)tournamentBrowser:(TournamentBrowser*)tournamentBroswer didUpdateServices:(NSArray*)services {
    [self setTournamentServices:services];

    // if just one, and not already connected, automatically connect
    BOOL connected = [[self session] connected];
    if([services count] == 1 && !connected) {
        // connect
        NSError* error;
        if([[self session] connectToTournamentService:services[0] error:&error]) {
            // store as current service
            [self setCurrentTournamentService:services[0]];

            // switch to clock screen automatically
            TBAppDelegate* appDelegate = (TBAppDelegate*)[[UIApplication sharedApplication] delegate];
            [[appDelegate rootViewController] setSelectedIndex:2];
        } else {
            [self presentError:error];
        }
    }
    // reload
    [[self tableView] reloadData];
}

@end
