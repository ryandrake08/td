//
//  TBTableViewController.h
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/31/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TournamentKit/TournamentKit.h"

@interface TBTableViewController : NSViewController

@property TournamentSession* session;

// initializer
- (instancetype)initWithNibName:(NSString*)nibName session:(TournamentSession*)sess;

@end
