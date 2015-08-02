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
                                           TournamentBrowserDelegate>

@property (nonatomic) IBOutlet TournamentSession* session;
@property (nonatomic) IBOutlet TournamentBrowser* tournamentBrowser;
@property (nonatomic) NSArray* netServices;
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

    if(cellService == currentService) {
        // pop disconnection actionsheet
        UIActionSheet* actionSheet = [[UIActionSheet alloc] initWithTitle:nil
                                                                 delegate:self
                                                        cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
                                                   destructiveButtonTitle:NSLocalizedString(@"Leave Game", nil)
                                                        otherButtonTitles:nil];
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
    if(buttonIndex == 0) {
        [[self session] disconnect];
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
