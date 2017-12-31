//
//  TBConnectToViewController.h
//  td
//
//  Created by Ryan Drake on 8/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TournamentSession.h"

@interface TBConnectToViewController : NSViewController

// the tournament session (model) object
@property (strong) IBOutlet TournamentSession* session;

@property (copy) NSString* address;
@property (assign) NSInteger port;

@end
