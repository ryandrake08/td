//
//  TournamentsViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentsViewController.h"
#import "TournamentDetailsViewController.h"
#import "TournamentKit_ios/TournamentKit.h"
#import "UIActionSheet+Blocks.h"

@interface TournamentsViewController () <TournamentSessionConnectionDelegate,
                                         TournamentDetailsViewControllerDelegate,
                                         UITableViewDelegate,
                                         UITableViewDataSource>

@property (nonatomic, strong) TournamentServerBrowser* browser;
@property (nonatomic, strong) TournamentServerInfo* connectedServer;
@end

@implementation TournamentsViewController
@synthesize browser;
@synthesize connectedServer;

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
        TournamentDetailsViewController* tournamentDetailsViewController = [navigationController viewControllers][0];
        [tournamentDetailsViewController setDelegate:self];
    }
}

#pragma mark - TournamentDetailsViewControllerDelegate

- (void)tournamentDetailsViewControllerDidCancel:(TournamentDetailsViewController*)controller {
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)tournamentDetailsViewController:(TournamentDetailsViewController*)controller didAddServer:(TournamentServerInfo*)server {
    [[self browser] addServer:server];
    NSIndexPath* indexPath = [NSIndexPath indexPathForRow:([[[self browser] serverList] count] - 1) inSection:0];
    [[self tableView] insertRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    [self dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return [[[self browser] serverList] count];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"ServerCell"];

    TournamentServerInfo* server = [[self browser] serverList][[indexPath row]];
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
    TournamentServerInfo* server = [[self browser] serverList][[indexPath row]];

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

- (void)tournamentSession:(TournamentSession*)session connectionStatusDidChange:(TournamentServerInfo*)server connected:(BOOL)connected {
    // update table view cell
    NSUInteger i = [[self browser] indexForServer:server];
    if(i != NSNotFound) {
        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:i inSection:0];
        [[self tableView] reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }

    if(connected) {
        [self setConnectedServer:server];

        // check authorization
        [session checkAuthorized];
    } else if(server == [self connectedServer]) {
        [self setConnectedServer:nil];
    }
}

- (void)tournamentSession:(TournamentSession*)session authorizationStatusDidChange:(TournamentServerInfo*)server authorized:(BOOL)authorized {
    NSUInteger i = [[self browser] indexForServer:server];
    if(i != NSNotFound) {
        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:i inSection:0];
        [[self tableView] reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

@end
