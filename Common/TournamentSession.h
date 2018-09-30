//
//  TournamentSession.h
//  td
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
@class TournamentService;

// Funding types (sync with enum funding_source_type_t in types.hpp)
#define kFundingTypeBuyin @0
#define kFundingTypeRebuy @1
#define kFundingTypeAddon @2

// Rebalance policies
#define kRebalanceManual @0
#define kRebalanceAutomatic @1
#define kRebalanceShootout @2

// Payout policies
#define kPayoutAutomatic @0
#define kPayoutForced @1
#define kPayoutManual @2

// Ante types
#define kAnteTypeNone @0
#define kAnteTypeTraditional @1
#define kAnteTypeBigBlind @2

// Standard 1 minute countdown
#define kActionClockRequestTime 60000

// Audio warning
#define kAudioWarningTime 60000

@protocol TournamentSessionDelegate;

@interface TournamentSession : NSObject

// cache the current tournament service, in order to reconnect
@property (nonatomic, strong) TournamentService* currentTournamentService;

// tournament configuration from session
@property (nonatomic, strong, readonly) NSMutableDictionary* configuration;

// tournament state from session
@property (nonatomic, strong, readonly) NSMutableDictionary* state;

// YES if connected
@property (nonatomic, assign, readonly) BOOL connected;

// YES if authorized
@property (nonatomic, assign, readonly) BOOL authorized;

// delegate to receive start, stop, and error callbacks
@property (nonatomic, weak) id <TournamentSessionDelegate> delegate;

// client identifier (used for authenticating with servers)
+ (NSNumber*)clientIdentifier;

// blind level naming utility
+ (NSArray*)blindLevelNamesForConfiguration:(NSDictionary*)config;

// connect to a tournament service
- (BOOL)connectToTournamentService:(TournamentService*)tournament error:(NSError**)error;
- (void)disconnect;

// configure the session with configuration by sending only changed keys
- (void)selectiveConfigure:(NSDictionary*)config withBlock:(void(^)(id))block;

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
- (void)genBlindLevelsRequest:(NSDictionary*)request withBlock:(void(^)(NSArray*))block;
- (void)fundPlayer:(id)playerId withFunding:(NSNumber*)sourceId;
- (void)planSeatingFor:(NSNumber*)expectedPlayers;
- (void)seatPlayer:(id)playerId withBlock:(void(^)(id playerId,NSNumber* tableNumber,NSNumber* seatNumber,BOOL alreadySeated))block;
- (void)unseatPlayer:(id)playerId withBlock:(void(^)(id playerId,NSNumber* tableNumber,NSNumber* seatNumber))block;
- (void)bustPlayer:(id)playerId withBlock:(void(^)(NSArray*))block;
- (void)rebalanceSeatingWithBlock:(void(^)(NSArray*))block;
- (void)quickSetupWithBlock:(void(^)(NSArray*))block;

// serialization
- (NSData*)dataWithResultsAsCSV;

@end

@protocol TournamentSessionDelegate <NSObject>
@optional
- (void)tournamentSessionDidBegin:(TournamentSession*)ts;
- (void)tournamentSessionDidEnd:(TournamentSession*)ts;
- (void)tournamentSession:(TournamentSession*)ts error:(NSError*)error;
@end
