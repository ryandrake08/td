//
//  TournamentsViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentsViewController.h"
#import "TournamentKit_ios/TournamentKit.h"

@interface TournamentsViewController () <TournamentConnectionDelegate>
{
    TournamentConnection* conn;
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

    // Do any additional setup after loading the view, typically from a nib.
    conn = [[TournamentConnection alloc] initWithHostname:@"localhost" port:25600];
    [conn setDelegate:self];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dealloc {
    [conn release];
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
    cell.detailTextLabel.text = @"Connected";

    return cell;
}

#pragma mark TournamentConnectionDelegate

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidConnect");
}

- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidDisconnect");
}

- (void)tournamentConnectionDidClose:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidClose");
}

- (void)tournamentConnection:(TournamentConnection*)tc didReceiveData:(id)json {
    NSLog(@"+++ tournamentConnectionDidReceiveData: %@", json);
}

- (void)tournamentConnection:(TournamentConnection*)tc error:(NSError*)error {
    NSLog(@"+++ tournamentConnectionError: %@", [error localizedDescription]);
}

@end
