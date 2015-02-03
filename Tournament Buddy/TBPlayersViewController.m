//
//  TBPlayersViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBPlayersViewController.h"

@interface TBPlayersViewController ()

@end

@implementation TBPlayersViewController

// initializer
- (instancetype)initWithSession:(TournamentSession*)sess {
    self = [super initWithNibName:@"TBPlayersView" session:sess];
    if(self) {
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

@end
