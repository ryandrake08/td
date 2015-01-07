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
{
    TournamentServerBrowser* browser;
}

@end

@implementation TournamentsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Be the session connection delegate
    [[TournamentSession sharedSession] setConnectionDelegate:self];

    // Initialize server list
    browser = [[TournamentServerBrowser alloc] init];

#if defined(DEBUG)
    // Test server for debugging
    TournamentServer* testServer = [[TournamentServer alloc] init];
    testServer.name = @"Local Debug";
    testServer.address = @"localhost";
    testServer.port = kDefaultTournamentServerPort;
    [browser addServer:testServer];
    [testServer release];
#endif
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dealloc {
    [browser release];
    [super dealloc];
}

#pragma mark - Segues

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([segue.identifier isEqualToString:@"AddServer"]) {
        UINavigationController* navigationController = segue.destinationViewController;
        TournamentDetailsViewController* tournamentDetailsViewController = [navigationController viewControllers][0];
        tournamentDetailsViewController.delegate = self;
    }
}

#pragma mark - TournamentDetailsViewControllerDelegate

- (void)tournamentDetailsViewControllerDidCancel:(TournamentDetailsViewController*)controller {
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)tournamentDetailsViewController:(TournamentDetailsViewController*)controller didAddServer:(TournamentServer*)server {
    [browser addServer:server];
    NSIndexPath* indexPath = [NSIndexPath indexPathForRow:([browser.serverList count] - 1) inSection:0];
    [self.tableView insertRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    [self dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return [browser.serverList count];
}

- (UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"ServerCell"];

    TournamentServer* server = (browser.serverList)[indexPath.row];
    cell.textLabel.text = server.name;
    if(server == [[TournamentSession sharedSession] currentServer]) {
        if(server.authorized) {
            cell.detailTextLabel.text = NSLocalizedString(@"Admin", nil);
        } else {
            cell.detailTextLabel.text = NSLocalizedString(@"Connected", nil);
        }
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    } else {
        cell.detailTextLabel.text = @"";
        cell.accessoryType = UITableViewCellAccessoryNone;
    }

    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    TournamentServer* server = (browser.serverList)[indexPath.row];

    if(server == [[TournamentSession sharedSession] currentServer]) {
        // pop connection actionsheet
        [UIActionSheet showInView:self.view
                        withTitle:nil
                cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
           destructiveButtonTitle:NSLocalizedString(@"Leave Game", nil)
                otherButtonTitles:nil
                         tapBlock:^(UIActionSheet* actionSheet, NSInteger buttonIndex) {
                             switch (buttonIndex) {
                                 case 0:
                                     server.authenticate = NO;
                                     [[TournamentSession sharedSession] disconnect];
                                     break;
                             }
                         }];
    } else {
        // pop connection actionsheet
        [UIActionSheet showInView:self.view
                        withTitle:nil
                cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
           destructiveButtonTitle:nil
                otherButtonTitles:@[NSLocalizedString(@"Player", nil), NSLocalizedString(@"Tournament Director", nil)]
                         tapBlock:^(UIActionSheet* actionSheet, NSInteger buttonIndex) {
                             switch (buttonIndex) {
                                 case 0:
                                     server.authenticate = NO;
                                     [[TournamentSession sharedSession] connectToServer:server];
                                     break;
                                 case 1:
                                     server.authenticate = YES;
                                     [[TournamentSession sharedSession] connectToServer:server];
                                     break;
                             }
                         }];
    }

    // deselect either way
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

#pragma mark TournamentSessionConnectionDelegate

- (void)tournamentSession:(TournamentSession*)session connectionStatusDidChange:(TournamentServer*)server connected:(BOOL)connected {
    // update table view cell
    NSUInteger i = [browser indexForServer:server];
    if(i != NSNotFound) {
        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:i inSection:0];
        [self.tableView reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }

    // query authentication status if necessary
    if(server == nil || server.authenticate) {
        [session checkAuthorized];
    }
}

- (void)tournamentSession:(TournamentSession*)session authorizationStatusDidChange:(TournamentServer*)server authorized:(BOOL)authorized {
    NSUInteger i = [browser indexForServer:server];
    if(i != NSNotFound) {
        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:i inSection:0];
        [self.tableView reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

@end
