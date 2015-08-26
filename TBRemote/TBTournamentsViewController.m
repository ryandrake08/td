//
//  TBTournamentsViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTournamentsViewController.h"
#import "TournamentBrowser.h"
#import "TournamentSession.h"
#import "TBAppDelegate.h"

#import "NSObject+FBKVOController.h"

@interface TBTournamentsViewController () <UITableViewDelegate,
                                           UITableViewDataSource,
                                           UIActionSheetDelegate,
                                           UIAlertViewDelegate,
                                           TournamentBrowserDelegate>

@property (nonatomic, strong) TournamentSession* session;
@property (nonatomic, strong) IBOutlet TournamentBrowser* tournamentBrowser;
@property (nonatomic, copy) NSArray* netServices;
@end

@implementation TBTournamentsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // initialize tournament browser
    [_tournamentBrowser search];

    // register for KVO
    [[self KVOController] observe:[self session] keyPath:NSStringFromSelector(@selector(isConnected)) options:0 block:^(id observer, id object, NSDictionary *change) {
        NSNetService* service = [object currentService];

        // update table view cell
        [observer reloadTableRowForService:service];

        if([object isConnected]) {
            // check authorization
            [object checkAuthorizedWithBlock:^(BOOL authorized) {
                [observer reloadTableRowForService:service];
            }];
        }
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
    return [[self netServices] count];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"ServiceCell"];

    NSNetService* cellService = [self netServices][[indexPath row]];
    NSNetService* currentService = [[self session] currentService];
    BOOL isConnected = [[self session] isConnected];
    BOOL isAuthorized = [[self session] isAuthorized];

    // always set name
    [[cell textLabel] setText:[cellService name]];

    // set checkmark and accessory text if connected
    if(cellService == currentService && isConnected) {
        if(isAuthorized) {
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
    NSNetService* cellService = [self netServices][[indexPath row]];
    NSNetService* currentService = [[self session] currentService];
    BOOL isConnected = [[self session] isConnected];
    BOOL isAuthorized = [[self session] isAuthorized];

    if(cellService == currentService && isConnected) {
        NSString* otherButtons = nil;
        if(!isAuthorized) {
            otherButtons = NSLocalizedString(@"Administer Game", nil);
        }

        // pop actionsheet
        UIActionSheet* actionSheet = [[UIActionSheet alloc] initWithTitle:nil
                                                                 delegate:self
                                                        cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
                                                   destructiveButtonTitle:NSLocalizedString(@"Leave Game", nil)
                                                        otherButtonTitles:otherButtons, nil];
        [actionSheet showInView:[self view]];
    } else {
        // connect
        [[self session] connectToService:cellService];
    }

    // deselect either way
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)reloadTableRowForService:(NSNetService*)service {
    NSUInteger i = [[self netServices] indexOfObject:service];
    if(i != NSNotFound) {
        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:i inSection:0];
        [[self tableView] reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

#pragma mark UIActionSheetDelegate

- (void)actionSheet:(UIActionSheet*)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
    if(buttonIndex == [actionSheet destructiveButtonIndex]) {
        [[self session] disconnect];
    } else if(buttonIndex == [actionSheet cancelButtonIndex]) {
        // do nothing
    } else if(buttonIndex == [actionSheet firstOtherButtonIndex]) {
        NSString* msg = [[NSString alloc] initWithFormat:NSLocalizedString(@"The tournament director needs to authorize this code: %@", nil), [TournamentSession clientIdentifier]];

        // present alert
        UIAlertView* alertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Administer Game", nil)
                                                            message:msg
                                                           delegate:self
                                                  cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
                                                  otherButtonTitles:NSLocalizedString(@"Try Again", nil), nil];
        [alertView show];
    }
}

#pragma UIAlertViewDelegate

- (void)alertView:(UIAlertView*)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex {
    if(buttonIndex == [alertView cancelButtonIndex]) {
        // do nothing
    } else if(buttonIndex == [alertView firstOtherButtonIndex]) {
        // check authorization
        [[self session] checkAuthorizedWithBlock:^(BOOL authorized) {
            NSNetService* currentService = [[self session] currentService];
            [self reloadTableRowForService:currentService];
        }];
    }
}

#pragma mark TournamentBroswerDelegate

- (void)tournamentBrowser:(TournamentBrowser*)tournamentBroswer didUpdateServices:(NSArray*)services {
    // filter out all but real NSNetServices
    NSMutableArray* newArray = [[NSMutableArray alloc] init];
    for(TournamentService* ts in services) {
        if([ts netService] != nil) {
            [newArray addObject:[ts netService]];
        }
    }
    [self setNetServices:newArray];

    // reload
    [[self tableView] reloadData];
}

@end
