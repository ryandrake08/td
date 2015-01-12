//
//  TBTournamentsViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTournamentsViewController.h"
#import "TBTournamentDetailsViewController.h"
#import "TournamentKit_ios/TournamentKit.h"
#import "UIActionSheet+Blocks.h"

@interface TBTournamentsViewController () <TBTournamentDetailsViewControllerDelegate,
                                           UITableViewDelegate,
                                           UITableViewDataSource>

@property (nonatomic) TournamentServerBrowser* browser;
@end

@implementation TBTournamentsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // register for KVO
    [[TournamentSession sharedSession] addObserver:self
                                        forKeyPath:NSStringFromSelector(@selector(isConnected))
                                           options:0
                                           context:NULL];

    // Initialize server list
    _browser = [[TournamentServerBrowser alloc] init];

#if defined(DEBUG)
    // Test server for debugging
    TournamentServerInfo* testServer = [[TournamentServerInfo alloc] init];
    [testServer setName:@"Local Debug"];
    [testServer setAddress:@"localhost"];
    [testServer setPort:kDefaultTournamentServerPort];
    [[self browser] addServer:testServer];
#endif
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dealloc {
    // unregister for KVO
    [[TournamentSession sharedSession] removeObserver:self forKeyPath:NSStringFromSelector(@selector(isConnected))];
}


#pragma mark - Segues

- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    if ([[segue identifier] isEqualToString:@"AddServer"]) {
        UINavigationController* navigationController = [segue destinationViewController];
        TBTournamentDetailsViewController* tournamentDetailsViewController = [navigationController viewControllers][0];
        [tournamentDetailsViewController setDelegate:self];
    }
}

#pragma mark - TBTournamentDetailsViewControllerDelegate

- (void)tournamentDetailsViewControllerDidCancel:(TBTournamentDetailsViewController*)controller {
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)tournamentDetailsViewController:(TBTournamentDetailsViewController*)controller didAddServer:(TournamentServerInfo*)server {
    [[self browser] addServer:server];
    NSIndexPath* indexPath = [NSIndexPath indexPathForRow:([[self browser] serverCount] - 1) inSection:0];
    [[self tableView] insertRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    [self dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return [[self browser] serverCount];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"ServerCell"];

    TournamentServerInfo* cellServer = [[self browser] serverForIndex:[indexPath row]];
    TournamentServerInfo* currentServer = [[TournamentSession sharedSession] currentServer];
    BOOL isConnected = [[TournamentSession sharedSession] isConnected];
    BOOL isAuthorized = [[TournamentSession sharedSession] isAuthorized];

    // always set name
    [[cell textLabel] setText:[cellServer name]];

    // set checkmark and accessory text if connected
    if(cellServer == currentServer && isConnected) {
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
    TournamentServerInfo* server = [[self browser] serverForIndex:[indexPath row]];

    if(server == [[TournamentSession sharedSession] currentServer]) {
        // pop disconnection actionsheet
        [UIActionSheet showInView:[self view]
                        withTitle:nil
                cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
           destructiveButtonTitle:NSLocalizedString(@"Leave Game", nil)
                otherButtonTitles:nil
                         tapBlock:^(UIActionSheet* actionSheet, NSInteger buttonIndex) {
                             switch (buttonIndex) {
                                 case 0:
                                     [[TournamentSession sharedSession] disconnect];
                                     break;
                             }
                         }];
    } else {
        // connect
        // TODO: activity indicator?
        [[TournamentSession sharedSession] connectToServer:server];
    }

    // deselect either way
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

#pragma mark KVO

- (void)reloadTableRowForServer:(TournamentServerInfo*)server {
    NSUInteger i = [[self browser] indexForServer:server];
    if(i != NSNotFound) {
        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:i inSection:0];
        [[self tableView] reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

- (void)observeValueForKeyPath:(NSString*)keyPath ofObject:(id)session change:(NSDictionary*)change context:(void*)context {
    if ([session isKindOfClass:[TournamentSession class]]) {
        if ([keyPath isEqualToString:NSStringFromSelector(@selector(isConnected))]) {
            TournamentServerInfo* server = [session currentServer];

            // update table view cell
            [self reloadTableRowForServer:server];

            if([session isConnected]) {
                // check authorization
                [session checkAuthorizedWithBlock:^(BOOL authorized) {
                    [self reloadTableRowForServer:server];
                }];
            }
        }
    }
}

@end
