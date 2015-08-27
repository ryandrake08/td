//
//  TBStatsViewController.h
//  td
//
//  Created by Ryan Drake on 8/27/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <TournamentSession.h>

@interface TBStatsViewController : NSViewController <NSTableViewDelegate, NSTableViewDataSource>

// global session
@property (strong) TournamentSession* session;

@end
