//
//  TBTournamentDetailsViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTournamentDetailsViewController.h"

@interface TBTournamentDetailsViewController ()

@property (nonatomic) IBOutlet UITextField* nameTextField;
@property (nonatomic) IBOutlet UITextField* addressTextField;
@property (nonatomic) IBOutlet UITextField* portTextField;

- (IBAction)cancel:(id)sender;
- (IBAction)done:(id)sender;

@end

@implementation TBTournamentDetailsViewController

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)cancel:(id)sender {
    [[self delegate] tournamentDetailsViewControllerDidCancel:self];
}

- (IBAction)done:(id)sender {
    TournamentServerInfo* server = [[TournamentServerInfo alloc] init];
    [server setName: [[self nameTextField] text]];
    [server setAddress: [[self addressTextField] text]];
    [server setPort: [[[self portTextField] text] integerValue]];
    [[self delegate] tournamentDetailsViewController:self didAddServer:server];
}

@end
