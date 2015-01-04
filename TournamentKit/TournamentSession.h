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

@protocol TournamentSessionConnectionDelegate;

@interface TournamentSession : NSObject

// delegate for connection-related messages
@property (nonatomic, assign) id<TournamentSessionConnectionDelegate> connectionDelegate;

// currently connected server, or nil if either connected locally or not connected
@property (nonatomic, readonly, retain) TournamentServer* currentServer;

// connect either locally through a unix socket or to a server
- (void)connectToLocal;
- (void)connectToServer:(TournamentServer*)server;
- (void)disconnect;

// client identifier (used for authenticating with servers)
+ (NSNumber*)clientIdentifier;

// singleton instance
+ (id)sharedSession;

@end

@protocol TournamentSessionConnectionDelegate <NSObject>

- (void)tournamentSession:(TournamentSession*)session connectionStatusDidChange:(TournamentServer*)server connected:(BOOL)connected;
- (void)tournamentSession:(TournamentSession*)session authorizationStatusDidChange:(TournamentServer*)server authorized:(BOOL)connected;

@end