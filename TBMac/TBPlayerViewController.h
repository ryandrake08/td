//
//  TBPlayerViewController.h
//  td
//
//  Created by Ryan Drake on 6/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TournamentSession.h"

@interface TBPlayerViewController : NSViewController

// the tournament session (model) object
@property (strong) TournamentSession* session;

@end
