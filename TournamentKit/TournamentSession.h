//
//  TournamentSession.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TournamentService.h"

// Funding types (sync with enum funding_source_type_t in types.hpp)
#define kFundingTypeBuyin @0
#define kFundingTypeRebuy @1
#define kFundingTypeAddon @2

@interface TournamentSession : NSObject

// currently connected server, or nil if either connected locally or not connected
@property (nonatomic, strong, readonly) NSNetService* currentService;

// YES if currently connected locally or to a server
@property (nonatomic, readonly, getter=isConnected) BOOL connected;

// YES if currently authorized with server
@property (nonatomic, readonly, getter=isAuthorized) BOOL authorized;

// tournament configuration
@property (nonatomic, strong, readonly) NSString* serverName;
@property (nonatomic, strong, readonly) NSString* serverVersion;
@property (nonatomic, strong, readonly) NSArray* authorizedClients;
@property (nonatomic, strong, readonly) NSString* name;
@property (nonatomic, strong, readonly) NSArray* players;
@property (nonatomic, strong, readonly) NSArray* blindLevels;
@property (nonatomic, strong, readonly) NSArray* availableChips;
@property (nonatomic, strong, readonly) NSString* costCurrency;
@property (nonatomic, strong, readonly) NSString* equityCurrency;
@property (nonatomic, strong, readonly) NSNumber* percentSeatsPaid;
@property (nonatomic, strong, readonly) NSNumber* roundPayouts;
@property (nonatomic, strong, readonly) NSNumber* payoutFlatness;
@property (nonatomic, strong, readonly) NSArray* fundingSources;
@property (nonatomic, strong, readonly) NSNumber* tableCapacity;
@property (nonatomic, strong, readonly) NSArray* manualPayouts;

// tournament state
@property (nonatomic, strong, readonly, getter=isRunning) NSNumber* running;
@property (nonatomic, strong, readonly) NSNumber* currentBlindLevel;
@property (nonatomic, strong, readonly) NSNumber* timeRemaining;
@property (nonatomic, strong, readonly) NSNumber* breakTimeRemaining;
@property (nonatomic, strong, readonly) NSNumber* actionClockTimeRemaining;
@property (nonatomic, strong, readonly) NSSet* buyins;
@property (nonatomic, strong, readonly) NSArray* entries;
@property (nonatomic, strong, readonly) NSArray* payouts;
@property (nonatomic, strong, readonly) NSNumber* totalChips;
@property (nonatomic, strong, readonly) NSNumber* totalCost;
@property (nonatomic, strong, readonly) NSNumber* totalCommission;
@property (nonatomic, strong, readonly) NSNumber* totalEquity;
@property (nonatomic, strong, readonly) NSArray* seats;
@property (nonatomic, strong, readonly) NSArray* playersFinished;
@property (nonatomic, strong, readonly) NSArray* emptySeats;
@property (nonatomic, strong, readonly) NSNumber* tables;
@property (nonatomic, strong, readonly) NSNumber* elapsedTime;

// derived tournament state
@property (nonatomic, readonly, getter=isPlanned) BOOL planned;
@property (nonatomic, readonly, getter=isOnBreak) BOOL onBreak;
@property (nonatomic, strong, readonly) NSString* clockText;
@property (nonatomic, strong, readonly) NSString* currentRoundNumberText;
@property (nonatomic, strong, readonly) NSString* currentGameText;
@property (nonatomic, strong, readonly) NSString* currentRoundText;
@property (nonatomic, strong, readonly) NSString* nextGameText;
@property (nonatomic, strong, readonly) NSString* nextRoundText;
@property (nonatomic, strong, readonly) NSString* playersLeftText;
@property (nonatomic, strong, readonly) NSString* entriesText;
@property (nonatomic, strong, readonly) NSString* averageStackText;
@property (nonatomic, strong, readonly) NSString* buyinText;
@property (nonatomic, strong, readonly) NSArray* results;
@property (nonatomic, strong, readonly) NSArray* seatedPlayers;
@property (nonatomic, strong, readonly) NSDictionary* playersLookup;
@property (nonatomic, strong, readonly) NSString* elapsedTimeText;

// number formatters
@property (nonatomic, strong, readonly) NSNumberFormatter* costFormatter;
@property (nonatomic, strong, readonly) NSNumberFormatter* equityFormatter;

// client identifier (used for authenticating with servers)
+ (NSNumber*)clientIdentifier;

// connect either locally through a unix socket or to a server
- (void)connectToLocalPath:(NSString*)path;
- (void)connectToAddress:(NSString*)address port:(NSInteger)port;
- (void)connectToService:(NSNetService*)service;
- (void)connect:(TournamentService*)tournament;
- (void)disconnect;

// configure the session with configuration by sending only changed keys
- (void)selectiveConfigureAndUpdate:(NSMutableDictionary*)config;

// tournament commands
- (void)checkAuthorizedWithBlock:(void(^)(BOOL))block;
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