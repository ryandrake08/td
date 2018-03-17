//
//  TBPlanViewController.h
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TBPlanViewController : NSViewController

// YES if warning text should be shown
@property (assign) BOOL enableWarning;

// Warning text to display
@property (strong) NSString* warningText;

// Number of players to plan for
@property (assign) NSInteger numberOfPlayers;

@end
