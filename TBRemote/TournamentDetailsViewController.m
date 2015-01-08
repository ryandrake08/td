//
//  TournamentDetailsViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentDetailsViewController.h"

@interface TournamentDetailsViewController ()

@property (nonatomic, strong) IBOutlet UITextField* nameTextField;
@property (nonatomic, strong) IBOutlet UITextField* addressTextField;
@property (nonatomic, strong) IBOutlet UITextField* portTextField;

- (IBAction)cancel:(id)sender;
- (IBAction)done:(id)sender;

@end

@implementation TournamentDetailsViewController

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
    TournamentServer* server = [[TournamentServer alloc] init];
    [server setName: [[self nameTextField] text]];
    [server setAddress: [[self addressTextField] text]];
    [server setPort: [[[self portTextField] text] integerValue]];
    [[self delegate] tournamentDetailsViewController:self didAddServer:server];
}

@end
