//
//  TBConnectToViewController.h
//  td
//
//  Created by Ryan Drake on 8/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

// Forward declare
@class TournamentSession;

@interface TBConnectToViewController : NSViewController

// the tournament session (model) object
@property (nonatomic, strong) TournamentSession* session;

@property (nonatomic, copy) NSString* address;
@property (nonatomic, assign) NSInteger port;

@end
