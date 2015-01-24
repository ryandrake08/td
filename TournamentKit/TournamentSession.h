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

// currently connected server, or nil if either connected locally or not connected
@property (nonatomic, readonly) TournamentServerInfo* currentServer;

// YES if currently connected locally or to a server
@property (nonatomic, readonly, assign, getter=isConnected) BOOL connected;

// YES if currently authorized with server
@property (nonatomic, readonly, assign, getter=isAuthorized) BOOL authorized;

// tournament configuration
@property (nonatomic, readonly) NSString* name;
@property (nonatomic, readonly) NSArray* players;
@property (nonatomic, readonly) NSArray* blindLevels;
@property (nonatomic, readonly) NSArray* availableChips;
@property (nonatomic, readonly) NSString* costCurrency;
@property (nonatomic, readonly) NSString* equityCurrency;
@property (nonatomic, readonly) NSNumber* percentSeatsPaid;
@property (nonatomic, readonly) NSArray* fundingSources;
@property (nonatomic, readonly) NSNumber* tableCapacity;

// tournament state
@property (nonatomic, readonly, getter=isRunning) NSNumber* running;
@property (nonatomic, readonly) NSNumber* currentBlindLevel;
@property (nonatomic, readonly) NSNumber* timeRemaining;
@property (nonatomic, readonly) NSNumber* breakTimeRemaining;
@property (nonatomic, readonly) NSNumber* actionClockTimeRemaining;
@property (nonatomic, readonly) NSSet* buyins;
@property (nonatomic, readonly) NSArray* payouts;
@property (nonatomic, readonly) NSNumber* totalChips;
@property (nonatomic, readonly) NSNumber* totalCost;
@property (nonatomic, readonly) NSNumber* totalCommission;
@property (nonatomic, readonly) NSNumber* totalEquity;
@property (nonatomic, readonly) NSArray* seats;
@property (nonatomic, readonly) NSArray* playersFinished;
@property (nonatomic, readonly) NSArray* emptySeats;
@property (nonatomic, readonly) NSNumber* tables;

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
- (void)togglePauseGame;
- (void)setPreviousLevelWithBlock:(void(^)(NSNumber*))block;
- (void)setNextLevelWithBlock:(void(^)(NSNumber*))block;
- (void)setActionClock:(NSNumber*)milliseconds;
- (void)genBlindLevels:(NSNumber*)count withDuration:(NSNumber*)durationMs breakDuration:(NSNumber*)breakDurationMs blindIncreaseFactor:(NSNumber*)increaseFactor;
- (void)fundPlayer:(NSNumber*)playerId withFunding:(NSNumber*)sourceId;
- (void)planSeatingFor:(NSNumber*)expectedPlayers;
- (void)seatPlayer:(NSNumber*)playerId withBlock:(void(^)(NSNumber*,NSNumber*,NSNumber*))block;
- (void)bustPlayer:(NSNumber*)playerId withBlock:(void(^)(NSArray*))block;

@end