//
//  TBConfigurationWindowController.h
//  td
//
//  Created by Ryan Drake on 8/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TournamentKit/TournamentKit.h"

@interface TBConfigurationWindowController : NSWindowController

// Configuration and session
@property (strong) NSMutableDictionary* configuration;
@property (strong) TournamentSession* session;

@end
