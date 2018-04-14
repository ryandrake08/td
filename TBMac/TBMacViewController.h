//
//  TBMacViewController.h
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class TournamentSession;

@interface TBMacViewController : NSViewController

// The session
@property (nonatomic, strong) TournamentSession* session;

// Returns the view appropriate for printing
- (NSView*)printableView;

@end
