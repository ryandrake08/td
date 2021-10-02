//
//  TBPlanViewController.h
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class TournamentSession;

@interface TBPlanViewController : NSViewController

// The session
@property (nonatomic, strong) TournamentSession* session;

// YES if warning text should be shown
@property (nonatomic, assign) BOOL enableWarning;

// Warning text to display
@property (nonatomic, strong) NSString* warningText;

// Number of players to plan for
@property (nonatomic, assign) NSUInteger numberOfPlayers;

@end
