//
//  TournamentSession.m
//  td
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentSession.h"
#import "TournamentConnection.h"
#import "NSDictionary+Changed.h"
#import "TBCurrencyNumberFormatter.h"

NSString* const TournamentSessionUpdatedNotification = @"TournamentSessionUpdatedNotification";

@interface TournamentSession() <TournamentConnectionDelegate>

// all tournament configuration and state
@property (nonatomic, strong) NSMutableDictionary* state;

// the connection object, handles networking and JSON serialization
@property (nonatomic, strong) TournamentConnection* connection;

// mapping between unique command and block to handle the command's response
@property (nonatomic, strong) NSMutableDictionary* blocksForCommands;

@end

@implementation TournamentSession

- (instancetype)init {
    if (self = [super init]) {
        _state = [[NSMutableDictionary alloc] init];
        _connection = [[TournamentConnection alloc] init];
        [_connection setDelegate:self];
        _blocksForCommands = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (BOOL)connectToLocalPath:(NSString*)path {
    [self disconnect];
    return [[self connection] connectToUnixSocketNamed:path];
}

- (BOOL)connectToAddress:(NSString *)address port:(NSInteger)port {
    [self disconnect];
    return [[self connection] connectToAddress:address andPort:port];
}

- (BOOL)connectToNetService:(NSNetService*)service {
    [self disconnect];
    return [[self connection] connectToNetService:service];
}

- (BOOL)connectToTournamentService:(TournamentService*)tournament {
    [self disconnect];
    return [[self connection] connectToTournamentService:tournament];
}

- (void)disconnect {
    [[self connection] close];
}

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString*)key {
    if([key isEqualToString:NSStringFromSelector(@selector(connection))]) {
        return NO;
    } else {
        return YES;
    }
}

// configure the session with configuration by sending only changed keys
- (void)selectiveConfigure:(NSDictionary*)config andUpdate:(NSMutableDictionary*)newConfig {
    NSLog(@"Synchronizing session");

    // only send parts of configuration that changed
    NSDictionary* configToSend = [config dictionaryWithChangesFromDictionary:[self state]];
    if([configToSend count] > 0) {
        NSLog(@"Sending %ld configuration items", (long)[configToSend count]);
        [self configure:configToSend withBlock:^(id json) {
            NSDictionary* differences = [json dictionaryWithChangesFromDictionary:newConfig];
            if([differences count] > 0) {
                NSLog(@"Updating existing config with %ld configuration items", (long)[differences count]);
                [newConfig addEntriesFromDictionary:differences];
            }
        }];
    }
}

#pragma mark Internal routines

// client identifier (used for authenticating with servers)
+ (NSNumber*)clientIdentifier {
    NSString* key = @"clientIdentifier";
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    // if we don't already have one
    if([defaults objectForKey:key] == nil) {
        // generate a new identifier
        u_int32_t cid = arc4random() % 90000 + 10000;
        [defaults setObject:@(cid) forKey:key];
    }
    return [defaults objectForKey:key];
}

// uniquely identify each command sent, so we can associate it with a block handler
+ (NSNumber*)commandKey {
    static int incrementingKey = 0;
    return @(incrementingKey++);
}

#pragma mark Tournament Commands

// send a command through TournamentConnection
- (void)sendCommand:(NSString*)cmd withData:(NSDictionary*)arg andBlock:(void(^)(id,NSString*))block {
    // add extra stuff to each command
    NSMutableDictionary* json = [NSMutableDictionary dictionaryWithDictionary:arg];

    // append to every command: authentication
    NSNumber* cid = [TournamentSession clientIdentifier];
    json[@"authenticate"] = cid;

    // append to every command: command key
    NSNumber* cmdkey = [TournamentSession commandKey];
    json[@"echo"] = cmdkey;

    // add block & command key to our dictionary, if block exists
    if(block != nil) {
        [self blocksForCommands][cmdkey] = block;
    }

    // send it through connection
    [[self connection] sendCommand:cmd withData:json];
}

- (void)checkAuthorizedWithBlock:(void(^)(BOOL))block {
    [self sendCommand:@"check_authorized" withData:nil andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"checkAuthorizedWithBlock: %@\n", error);
        } else {
            // set internal state
            [self state][@"authorized"] = json[@"authorized"];
            // handle authorization check
            if(block != nil) {
                block([json[@"authorized"] boolValue]);
            }
        }
    }];
}

- (void)getStateWithBlock:(void(^)(id))block {
    [self sendCommand:@"get_state" withData:nil andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"getStateWithBlock: %@\n", error);
        } else {
            // handle config response
            if(block != nil) {
                block(json);
            }
        }
    }];
}


- (void)getConfigWithBlock:(void(^)(id))block {
    [self sendCommand:@"get_config" withData:nil andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"getConfigWithBlock: %@\n", error);
        } else {
            // handle config response
            if(block != nil) {
                block(json);
            }
        }
    }];
}

- (void)configure:(id)config withBlock:(void(^)(id))block {
    [self sendCommand:@"configure" withData:config andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"configureWithBlock: %@\n", error);
        } else {
            // handle config response
            if(block != nil) {
                block(json);
            }
        }
    }];
}

- (void)startGameAt:(NSDate*)datetime {
    [self sendCommand:@"start_game" withData:@{@"start_at" : datetime} andBlock:nil];
}

- (void)startGame {
    [self sendCommand:@"start_game" withData:nil andBlock:nil];
}

- (void)stopGame {
    [self sendCommand:@"stop_game" withData:nil andBlock:nil];
}

- (void)resumeGame {
    [self sendCommand:@"resume_game" withData:nil andBlock:nil];
}

- (void)pauseGame {
    [self sendCommand:@"pause_game" withData:nil andBlock:nil];
}

- (void)togglePauseGame {
    [self sendCommand:@"toggle_pause_game" withData:nil andBlock:nil];
}

- (void)setPreviousLevelWithBlock:(void(^)(NSNumber*))block {
    [self sendCommand:@"set_previous_level" withData:nil andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"setPreviousLevelWithBlock: %@\n", error);
        } else {
            // handle blind level change
            if(block != nil) {
                block(json[@"blind_level_changed"]);
            }
        }
    }];
}

- (void)setNextLevelWithBlock:(void(^)(NSNumber*))block {
    [self sendCommand:@"set_next_level" withData:nil andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"setNextLevelWithBlock: %@\n", error);
        } else {
            // handle blind level change
            if(block != nil) {
                block(json[@"blind_level_changed"]);
            }
        }
    }];
}

- (void)setActionClock:(NSNumber*)milliseconds {
    [self sendCommand:@"set_action_clock" withData:@{@"duration" : milliseconds} andBlock:nil];
}

- (void)clearActionClock {
    [self sendCommand:@"set_action_clock" withData:nil andBlock:nil];
}

- (void)genBlindLevels:(NSNumber*)count withDuration:(NSNumber*)durationMs breakDuration:(NSNumber*)breakDurationMs blindIncreaseFactor:(NSNumber*)increaseFactor {
    [self sendCommand:@"gen_blind_levels" withData:@{@"count" : count, @"duration" : durationMs, @"break_duration" : breakDurationMs, @"blind_increase_factor" : increaseFactor } andBlock:nil];
}

- (void)fundPlayer:(id)playerId withFunding:(NSNumber*)sourceId {
    [self sendCommand:@"fund_player" withData:@{@"player_id" : playerId, @"source_id" : sourceId} andBlock:nil];
}

- (void)planSeatingFor:(NSNumber*)expectedPlayers {
    [self sendCommand:@"plan_seating" withData:@{@"max_expected_players" : expectedPlayers} andBlock:nil];
}

- (void)seatPlayer:(id)playerId withBlock:(void(^)(id,NSNumber*,NSNumber*))block {
    [self sendCommand:@"seat_player" withData:@{@"player_id" : playerId} andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"seatPlayerWithBlock: %@\n", error);
        } else {
            // handle seated player
            id playerSeated = json[@"player_seated"];
            if(playerSeated) {
                if(block != nil) {
                    block(playerSeated[@"player_id"], playerSeated[@"table_number"], playerSeated[@"seat_number"]);
                }
            } else {
                if(block != nil) {
                    block(nil, nil, nil);
                }
            }
        }
    }];
}

- (void)unseatPlayer:(id)playerId withBlock:(void(^)(id,NSNumber*,NSNumber*))block {
    [self sendCommand:@"unseat_player" withData:@{@"player_id" : playerId} andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"unseatPlayerWithBlock: %@\n", error);
        } else {
            // handle seated player
            id playerUnseated = json[@"player_unseated"];
            if(playerUnseated) {
                if(block != nil) {
                    block(playerUnseated[@"player_id"], playerUnseated[@"table_number"], playerUnseated[@"seat_number"]);
                }
            } else {
                if(block != nil) {
                    block(nil, nil, nil);
                }
            }
        }
    }];
}

- (void)bustPlayer:(id)playerId withBlock:(void(^)(NSArray*))block {
    [self sendCommand:@"bust_player" withData:@{@"player_id" : playerId} andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"bustPlayerWithBlock: %@\n", error);
        } else {
            // handle player movement
            if(block != nil) {
                block(json[@"players_moved"]);
            }
        }
    }];
}

#pragma mark Tournament Messages

- (void)handleMessage:(NSMutableDictionary*)json fromConnection:(TournamentConnection*)tc {
    // look for command key
    NSNumber* cmdkey = json[@"echo"];
    if(cmdkey) {
        // remove command key response
        [json removeObjectForKey:@"echo"];

        // look up block for command key
        void (^block)(id,NSString*) = [self blocksForCommands][cmdkey];
        if(block) {
            // if it's a command with a handler block, call that block
            block(json, json[@"error"]);

            // remove it from our dictionary
            [[self blocksForCommands] removeObjectForKey:cmdkey];
        }
    } else {
        // replace only state that changed
        NSDictionary* update = [json dictionaryWithChangesFromDictionary:[self state]];
        [[self state] addEntriesFromDictionary:update];

        // post notification
        [[NSNotificationCenter defaultCenter] postNotificationName:TournamentSessionUpdatedNotification object:update];
    }
}

#pragma mark TournamentConnectionDelegate

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc {
    NSAssert([self connection] == tc, @"Unexpected connection from %@", tc);
    // set state
    [self state][@"connected"] = @YES;

    // always check if we're authorized right away
    [self checkAuthorizedWithBlock:nil];
}

- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc {
    NSAssert([self connection] == tc, @"Unexpected disconnection from %@", tc);
    [self disconnect];
}

- (void)tournamentConnectionDidClose:(TournamentConnection*)tc {
    NSAssert([self connection] == tc, @"Unexpected close from %@", tc);
    // set state
    [[self state] removeAllObjects];
    [self state][@"connected"] = @NO;
}

- (void)tournamentConnection:(TournamentConnection*)tc didReceiveData:(id)json {
    NSAssert([self connection] == tc, @"Unexpected data from %@", tc);
    [self handleMessage:json fromConnection:tc];
}

- (void)tournamentConnection:(TournamentConnection*)tc error:(NSError*)error {
    NSAssert([self connection] == tc, @"Unexpected error from %@", tc);
    [self disconnect];
}

// utility (TODO: better place for this?)

+ (NSArray*)namesForBlindLevels:(NSArray*)blindLevels {
    NSMutableArray* names = [[NSMutableArray alloc] initWithObjects:NSLocalizedString(@"Tournament Start", nil), nil];
    for(NSInteger i=1; i<[blindLevels count]; i++) {
        [names addObject:[NSString stringWithFormat:NSLocalizedString(@"Round %ld", @"Numbered blind level"), i]];
    }
    return names;
}

@end
