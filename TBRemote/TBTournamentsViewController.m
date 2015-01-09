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

@interface TBTournamentsViewController () <TournamentSessionConnectionDelegate,
                                           TBTournamentDetailsViewControllerDelegate,
                                           UITableViewDelegate,
                                           UITableViewDataSource>

@property (nonatomic, strong) TournamentServerBrowser* browser;
@end

@implementation TBTournamentsViewController
@synthesize browser;

- (void)viewDidLoad {
    [super viewDidLoad];

    // Be the session connection delegate
    [[TournamentSession sharedSession] setConnectionDelegate:self];

    // Initialize server list
    browser = [[TournamentServerBrowser alloc] init];

#if defined(DEBUG)
    // Test server for debugging
    TournamentServerInfo* testServer = [[TournamentServerInfo alloc] init];
    [testServer setName:@"Local Debug"];
    [testServer setAddress:@"localhost"];
    [testServer setPort:kDefaultTournamentServerPort ];
    [[self browser] addServer:testServer];
#endif
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
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

    TournamentServerInfo* server = [[self browser] serverForIndex:[indexPath row]];
    [[cell textLabel] setText:[server name]];
    if(server == [[TournamentSession sharedSession] currentServer]) {
        if([[TournamentSession sharedSession] isAuthorized]) {
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

#pragma mark TournamentSessionConnectionDelegate

- (void)reloadTableRowForServer:(TournamentServerInfo*)server {
    NSUInteger i = [[self browser] indexForServer:server];
    if(i != NSNotFound) {
        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:i inSection:0];
        [[self tableView] reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

- (void)tournamentSession:(TournamentSession*)session connectionStatusDidChange:(TournamentServerInfo*)server connected:(BOOL)connected {
    // update table view cell
    [self reloadTableRowForServer:server];

    if(connected) {
        // check authorization
        [session checkAuthorizedWithBlock:^(BOOL authorized) {
            [self reloadTableRowForServer:server];
        }];
    }
}

@end
