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
@property (nonatomic, strong) NSNetService* currentService;
@property (nonatomic, copy) NSArray* netServices;
@end

@implementation TBTournamentsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // initialize tournament browser
    [[self tournamentBrowser] search];

    // register for KVO
    [[self KVOController] observe:[[self session] state] keyPaths:@[@"connected", @"authorized"] options:0 block:^(id observer, id object, NSDictionary *change) {
        // update table view cell
        [observer reloadTableRowForService:[self currentService]];
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
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"ServiceCell" forIndexPath:indexPath];

    NSNetService* cellService = [self netServices][[indexPath row]];
    BOOL connected = [[[self session] state][@"connected"] boolValue];
    BOOL authorized = [[[self session] state][@"authorized"] boolValue];

    // always set name
    [[cell textLabel] setText:[cellService name]];

    // set checkmark and accessory text if connected
    if(cellService == [self currentService] && connected) {
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
    NSNetService* cellService = [self netServices][[indexPath row]];
    BOOL connected = [[[self session] state][@"connected"] boolValue];
    BOOL authorized = [[[self session] state][@"authorized"] boolValue];

    if(cellService == [self currentService] && connected) {
        NSString* otherButtons = nil;
        if(!authorized) {
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
        [self setCurrentService:cellService];
        if(![[self session] connectToNetService:cellService]) {
            // TODO: handle error
        }
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
        [self setCurrentService:nil];
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

#pragma mark NSArrayUIAlertViewDelegate

- (void)alertView:(UIAlertView*)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex {
    if(buttonIndex == [alertView cancelButtonIndex]) {
        // do nothing
    } else if(buttonIndex == [alertView firstOtherButtonIndex]) {
        // check authorization
        [[self session] checkAuthorizedWithBlock:nil];
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

    // if just one, automatically connect
    if([newArray count] == 1) {
        // connect
        [self setCurrentService:newArray[0]];
        if([[self session] connectToNetService:newArray[0]]) {
            TBAppDelegate* appDelegate = (TBAppDelegate*)[[UIApplication sharedApplication] delegate];
            [(UITabBarController*)[[appDelegate window] rootViewController] setSelectedIndex:1];
        } else {
            // TODO: handle error
        }
    }
    // reload
    [[self tableView] reloadData];
}

@end
