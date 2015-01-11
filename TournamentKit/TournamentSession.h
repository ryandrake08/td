//
//  TournamentSession.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TournamentServerInfo.h"

#define kDefaultTournamentServerPort 25600

@interface TournamentSession : NSObject

// YES if currently connected locally or to a server
@property (nonatomic, readonly, assign, getter=isConnected) BOOL connected;

// currently connected server, or nil if either connected locally or not connected
@property (nonatomic, readonly, strong) TournamentServerInfo* currentServer;

// YES if currently authorized with server
@property (nonatomic, readonly, assign, getter=isAuthorized) BOOL authorized;

// connect either locally through a unix socket or to a server
- (void)connectToLocal;
- (void)connectToServer:(TournamentServerInfo*)server;
- (void)disconnect;

// tournament commands
- (void)checkAuthorizedWithBlock:(void(^)(BOOL))block;
- (void)authorize:(NSNumber*)clientId withBlock:(void(^)(NSNumber*))block;
- (void)startGameAt:(NSDate*)datetime;
- (void)stopGame;
- (void)resumeGame;
- (void)pauseGame;
- (void)setPreviousLevelWithBlock:(void(^)(NSNumber*))block;
- (void)setNextLevelWithBlock:(void(^)(NSNumber*))block;
- (void)setActonClock:(NSNumber*)milliseconds;
- (void)genBlindLevelsCount:(NSNumber*)count withDuration:(NSNumber*)milliseconds;
- (void)resetFunding;
- (void)fundPlayer:(NSNumber*)playerId withFunding:(NSNumber*)sourceId;
- (void)planSeatingFor:(NSNumber*)expectedPlayers;
- (void)seatPlayer:(NSNumber*)playerId withBlock:(void(^)(NSNumber*,NSNumber*,NSNumber*))block;
- (void)bustPlayer:(NSNumber*)playerId withBlock:(void(^)(NSArray*))block;

// singleton instance
+ (instancetype)sharedSession;

@end