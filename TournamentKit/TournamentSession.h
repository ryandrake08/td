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

// true if currently authorized with server
@property (nonatomic, readonly, assign) BOOL isAuthorized;

// connect either locally through a unix socket or to a server
- (void)connectToLocal;
- (void)connectToServer:(TournamentServer*)server;
- (void)disconnect;

// tournament commands
- (void)checkAuthorized;
- (void)authorize:(NSNumber*)clientId;
- (void)startGameAt:(NSDate*)datetime;
- (void)stopGame;
- (void)resumeGame;
- (void)pauseGame;
- (void)setPreviousLevel;
- (void)setNextLevel;
- (void)setActonClock:(NSNumber*)milliseconds;
- (void)genBlindLevelsCount:(NSNumber*)count withDuration:(NSNumber*)milliseconds;
- (void)resetFunding;
- (void)fundPlayer:(NSNumber*)playerId withFunding:(NSNumber*)sourceId;
- (void)planSeatingFor:(NSNumber*)expectedPlayers;
- (void)seatPlayer:(NSNumber*)playerId;
- (void)bustPlayer:(NSNumber*)playerId;

// singleton instance
+ (id)sharedSession;

@end

@protocol TournamentSessionConnectionDelegate <NSObject>

- (void)tournamentSession:(TournamentSession*)session connectionStatusDidChange:(TournamentServer*)server connected:(BOOL)connected;
- (void)tournamentSession:(TournamentSession*)session authorizationStatusDidChange:(TournamentServer*)server authorized:(BOOL)connected;

@end