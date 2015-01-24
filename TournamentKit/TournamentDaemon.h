//
//  TournamentDaemon.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/4/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface TournamentDaemon : NSObject

// start the daemon, pre-authorizing given client code, returning local unix socket path
- (NSString*)startWithAuthCode:(NSNumber*)code;

// publish over Bojour using name
- (void)publishWithName:(NSString*)name;

// stop the daemon (and stop publishing if doing so)
- (void)stop;

@end
