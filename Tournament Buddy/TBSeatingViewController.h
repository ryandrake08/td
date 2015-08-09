//
//  TBSeatingViewController.h
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTableViewController.h"

@interface TBSeatingViewController : NSViewController

// global configuration
@property (strong) NSMutableDictionary* configuration;

// global session
@property (strong) TournamentSession* session;

@end
