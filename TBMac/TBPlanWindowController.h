//
//  TBPlanWindowController.h
//  td
//
//  Created by Ryan Drake on 5/15/16.
//  Copyright © 2016 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TBPlanWindowController : NSWindowController

// number of players to plan for
@property (assign) NSInteger numberOfPlayers;

// YES if warning text should be shown
@property (assign) BOOL enableWarning;

@end
