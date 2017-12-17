//
//  TournamentSession.h
//  td
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

// Standard 1 minute countdown
#define kActionClockRequestTime 60000

// Audio warning
#define kAudioWarningTime 60000

// Notification
extern NSString* const TournamentSessionUpdatedNotification;

@interface TournamentSession : NSObject

// all tournament configuration and state
@property (nonatomic, strong, readonly) NSMutableDictionary* state;

// client identifier (used for authenticating with servers)
+ (NSNumber*)clientIdentifier;

// connect either locally through a unix socket or to a server
- (BOOL)connectToLocalPath:(NSString*)path;
- (BOOL)connectToAddress:(NSString*)address port:(NSInteger)port;
- (BOOL)connectToNetService:(NSNetService*)service;
- (BOOL)connectToTournamentService:(TournamentService*)tournament;
- (void)disconnect;

// configure the session with configuration by sending only changed keys
- (void)selectiveConfigure:(NSDictionary*)config andUpdate:(NSMutableDictionary*)newConfig;

// tournament commands
- (void)checkAuthorizedWithBlock:(void(^)(BOOL))block;
- (void)getStateWithBlock:(void(^)(id))block;
- (void)getConfigWithBlock:(void(^)(id))block;
- (void)configure:(id)config withBlock:(void(^)(id))block;
- (void)startGameAt:(NSDate*)datetime;
- (void)startGame;
- (void)stopGame;
- (void)resumeGame;
- (void)pauseGame;
- (void)togglePauseGame;
- (void)setPreviousLevelWithBlock:(void(^)(NSNumber*))block;
- (void)setNextLevelWithBlock:(void(^)(NSNumber*))block;
- (void)setActionClock:(NSNumber*)milliseconds;
- (void)clearActionClock;
- (void)genBlindLevels:(NSNumber*)count withDuration:(NSNumber*)durationMs breakDuration:(NSNumber*)breakDurationMs blindIncreaseFactor:(NSNumber*)increaseFactor;
- (void)fundPlayer:(id)playerId withFunding:(NSNumber*)sourceId;
- (void)planSeatingFor:(NSNumber*)expectedPlayers;
- (void)seatPlayer:(id)playerId withBlock:(void(^)(id playerId,NSNumber* tableNumber,NSNumber* seatNumber,BOOL alreadySeated))block;
- (void)unseatPlayer:(id)playerId withBlock:(void(^)(id playerId,NSNumber* tableNumber,NSNumber* seatNumber))block;
- (void)bustPlayer:(id)playerId withBlock:(void(^)(NSArray*))block;

// utility (TODO: better place for this?)
+ (NSArray*) namesForBlindLevels:(NSArray*)blindLevels;

@end
