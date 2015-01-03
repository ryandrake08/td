//
//  FirstViewController.m
//  TB Remote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "FirstViewController.h"
#import "TournamentKit_ios/TournamentKit.h"

@interface FirstViewController ()
{
    TournamentConnection* conn;
}
@end

@implementation FirstViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    conn = [[TournamentConnection alloc] initWithHostname:@"localhost" port:25600];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dealloc {
    [conn release];
    [super dealloc];
}

@end
