//
//  TournamentSession.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TournamentService.h"

@interface TournamentSession : NSObject

// currently connected server, or nil if either connected locally or not connected
@property (nonatomic, readonly) NSNetService* currentService;

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
@property (nonatomic, readonly) NSNumber* roundPayouts;
@property (nonatomic, readonly) NSNumber* payoutFlatness;
@property (nonatomic, readonly) NSArray* fundingSources;
@property (nonatomic, readonly) NSNumber* tableCapacity;
@property (nonatomic, readonly) NSArray* manualPayouts;

// tournament state
@property (nonatomic, readonly, getter=isRunning) NSNumber* running;
@property (nonatomic, readonly) NSNumber* currentBlindLevel;
@property (nonatomic, readonly) NSNumber* timeRemaining;
@property (nonatomic, readonly) NSNumber* breakTimeRemaining;
@property (nonatomic, readonly) NSNumber* actionClockTimeRemaining;
@property (nonatomic, readonly) NSSet* buyins;
@property (nonatomic, readonly) NSArray* entries;
@property (nonatomic, readonly) NSArray* payouts;
@property (nonatomic, readonly) NSNumber* totalChips;
@property (nonatomic, readonly) NSNumber* totalCost;
@property (nonatomic, readonly) NSNumber* totalCommission;
@property (nonatomic, readonly) NSNumber* totalEquity;
@property (nonatomic, readonly) NSArray* seats;
@property (nonatomic, readonly) NSArray* playersFinished;
@property (nonatomic, readonly) NSArray* emptySeats;
@property (nonatomic, readonly) NSNumber* tables;

// derived tournament state
@property (nonatomic, readonly, getter=isPlanned) BOOL planned;
@property (nonatomic, readonly, getter=isOnBreak) BOOL onBreak;
@property (nonatomic, readonly) NSString* clockText;
@property (nonatomic, readonly) NSString* currentGameText;
@property (nonatomic, readonly) NSString* currentRoundText;
@property (nonatomic, readonly) NSString* nextGameText;
@property (nonatomic, readonly) NSString* nextRoundText;
@property (nonatomic, readonly) NSString* playersLeftText;
@property (nonatomic, readonly) NSString* entriesText;
@property (nonatomic, readonly) NSString* averageStackText;
@property (nonatomic, readonly) NSArray* results;
@property (nonatomic, readonly) NSSet* seatedPlayers;
@property (nonatomic, readonly) NSArray* blindLevelNames;
@property (nonatomic, readonly) NSDictionary* playersLookup;

// client identifier (used for authenticating with servers)
+ (NSNumber*)clientIdentifier;

// connect either locally through a unix socket or to a server
- (void)connectToLocalPath:(NSString*)path;
- (void)connectToAddress:(NSString*)address port:(NSInteger)port;
- (void)connectToService:(NSNetService*)service;
- (void)connect:(TournamentService*)tournament;
- (void)disconnect;

// get current configuration
- (id)currentConfiguration;

// tournament commands
- (void)checkAuthorizedWithBlock:(void(^)(BOOL))block;
- (void)authorize:(NSNumber*)clientId withBlock:(void(^)(NSNumber*))block;
- (void)getStateWithBlock:(void(^)(id))block;
- (void)getConfigWithBlock:(void(^)(id))block;
- (void)configure:(id)config withBlock:(void(^)(id))block;
- (void)startGameAt:(NSDate*)datetime;
- (void)stopGame;
- (void)resumeGame;
- (void)pauseGame;
- (void)togglePauseGame;
- (void)setPreviousLevelWithBlock:(void(^)(NSNumber*))block;
- (void)setNextLevelWithBlock:(void(^)(NSNumber*))block;
- (void)setActionClock:(NSNumber*)milliseconds;
- (void)genBlindLevels:(NSNumber*)count withDuration:(NSNumber*)durationMs breakDuration:(NSNumber*)breakDurationMs blindIncreaseFactor:(NSNumber*)increaseFactor;
- (void)fundPlayer:(id)playerId withFunding:(NSNumber*)sourceId;
- (void)planSeatingFor:(NSNumber*)expectedPlayers;
- (void)seatPlayer:(id)playerId withBlock:(void(^)(id playerId,NSNumber* tableNumber,NSNumber* seatNumber))block;
- (void)unseatPlayer:(id)playerId withBlock:(void(^)(id playerId,NSNumber* tableNumber,NSNumber* seatNumber))block;
- (void)bustPlayer:(id)playerId withBlock:(void(^)(NSArray*))block;

@end