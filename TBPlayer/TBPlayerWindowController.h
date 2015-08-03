//
//  TBPlayerWindowController.h
//  td
//
//  Created by Ryan Drake on 6/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TournamentKit/TournamentKit.h"

@interface TBPlayerWindowController : NSWindowController

// the tournament session (model) object
@property TournamentSession* session;

// enter and exit full screen
- (void)enterFullScreenModeIfPossible;
- (void)exitFullScreenModeIfPossible;

@end
