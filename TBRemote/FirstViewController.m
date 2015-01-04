//
//  FirstViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "FirstViewController.h"
#import "TournamentKit_ios/TournamentKit.h"

@interface FirstViewController () <TournamentConnectionDelegate>
{
    TournamentConnection* conn;
}
@end

@implementation FirstViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
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
    [super dealloc];
}

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
    NSLog(@"+++ tournamentConnectionError");
}

@end
