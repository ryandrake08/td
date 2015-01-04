//
//  TournamentSession.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TournamentServer.h"

#define kDefaultTournamentServerPort 25600

@interface TournamentSession : NSObject

@property (nonatomic, readonly, retain) TournamentServer* server;

- (void)connectLocally;
- (void)connectToServer:(TournamentServer*)server;

// Singleton instance
+ (id)sharedSession;

@end
