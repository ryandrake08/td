//
//  TBSeatingViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSeatingViewController.h"

@interface TBSeatingViewController ()

@end

@implementation TBSeatingViewController

// initializer
- (instancetype)initWithSession:(TournamentSession*)sess {
    self = [super initWithNibName:@"TBSeatingView" session:sess];
    if(self) {
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

@end
