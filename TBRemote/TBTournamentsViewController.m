//
//  TBTournamentsViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTournamentsViewController.h"
#import "TournamentKit/TournamentKit.h"
#import "TBAppDelegate.h"

#import "NSObject+FBKVOController.h"

@interface TBTournamentsViewController () <UITableViewDelegate,
                                           UITableViewDataSource,
                                           UIActionSheetDelegate,
                                           NSNetServiceBrowserDelegate>

@property (nonatomic) TournamentSession* session;
@property (nonatomic) NSMutableArray* serviceList;
@property (nonatomic) NSNetServiceBrowser* serviceBrowser;
@end

@implementation TBTournamentsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // create service list
    _serviceList = [[NSMutableArray alloc] init];

    // initialize service browser
    _serviceBrowser = [[NSNetServiceBrowser alloc] init];

    // configure Service browser
    [self.serviceBrowser setDelegate:self];
    [self.serviceBrowser searchForServicesOfType:@"_tournbuddy._tcp." inDomain:@"local."];

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
    [[self serviceBrowser] stop];
    [[self serviceBrowser] setDelegate:nil];
    [self setServiceBrowser:nil];
}


#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return [[self serviceList] count];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"ServiceCell"];

    NSNetService* cellService = [[self serviceList] objectAtIndex:[indexPath row]];
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
    NSNetService* service = [[self serviceList] objectAtIndex:[indexPath row]];

    if(service == [[self session] currentService]) {
        // pop disconnection actionsheet
        UIActionSheet* actionSheet = [[UIActionSheet alloc] initWithTitle:nil
                                                                 delegate:self
                                                        cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
                                                   destructiveButtonTitle:NSLocalizedString(@"Leave Game", nil)
                                                        otherButtonTitles:nil];
        [actionSheet showInView:[self view]];
    } else {
        // connect
        [[self session] connectToService:service];
    }

    // deselect either way
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)reloadTableRowForService:(NSNetService*)service {
    NSUInteger i = [[self serviceList] indexOfObject:service];
    if(i != NSNotFound) {
        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:i inSection:0];
        [[self tableView] reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

#pragma mark UIActionSheetDelegate

- (void)actionSheet:(UIActionSheet*)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
    if(buttonIndex == 0) {
        [[self session] disconnect];
    }
}

#pragma mark NSNetServiceDelegate

- (void)netServiceBrowser:(NSNetServiceBrowser*)serviceBrowser didFindService:(NSNetService*)service moreComing:(BOOL)moreComing {
    [[self serviceList] addObject:service];

    if(!moreComing) {
        [[self tableView] reloadData];
    }
}

- (void)netServiceBrowser:(NSNetServiceBrowser*)serviceBrowser didRemoveService:(NSNetService*)service moreComing:(BOOL)moreComing {
    [[self serviceList] removeObject:service];

    if(!moreComing) {
        [[self tableView] reloadData];
    }
}

@end
