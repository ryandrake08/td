//
//  TournamentsViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentsViewController.h"
#import "TournamentKit_ios/TournamentKit.h"

@interface TournamentsViewController ()
{
    TournamentServerBrowser* browser;
}

@end

@implementation TournamentsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

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
    if(server == [[TournamentSession sharedSession] server]) {
        cell.detailTextLabel.text = @"Connected";
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
    [[TournamentSession sharedSession] connectToServer:server];
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
