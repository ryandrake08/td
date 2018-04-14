//
//  TBActionClockViewController.h
//  td
//
//  Created by Ryan Drake on 12/14/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class TournamentSession;

@interface TBActionClockViewController : NSViewController

// The session
@property (nonatomic, strong) TournamentSession* session;

@end
