//
//  TournamentSession.m
//  td
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentSession.h"
#import "NSDictionary+Changed.h"
#import "TBClockDateComponentsFormatter.h"
#import "TBCurrencyNumberFormatter.h"
#import "TBError.h"
#import "TournamentConnection.h"
#import "TournamentService.h"

@interface TournamentSession() <TournamentConnectionDelegate>

// cache the current tournament service, in order to reconnect
@property (nonatomic, strong) TournamentService* currentTournamentService;

// tournament state from session
@property (nonatomic, strong) NSMutableDictionary* state;

// YES if connected
@property (nonatomic, assign) BOOL connected;

// YES if authorized
@property (nonatomic, assign) BOOL authorized;

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

- (BOOL)connectToTournamentService:(TournamentService*)tournament error:(NSError**)error {
    [self disconnect];
    if([[self connection] connectToTournamentService:tournament error:error] == YES) {
        // cache service for later reconnection
        [self setCurrentTournamentService:tournament];
        return YES;
    }

    return NO;
}

- (void)disconnect {
    [[self connection] close];
    [self setCurrentTournamentService:nil];
}

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString*)key {
    if([key isEqualToString:NSStringFromSelector(@selector(connection))]) {
        return NO;
    } else {
        return YES;
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

// array of blind level names given configuration
+ (NSArray*)blindLevelNamesForConfiguration:(NSDictionary*)config {
    NSMutableArray* names = [[NSMutableArray alloc] initWithObjects:NSLocalizedString(@"Start", @"The start of the tournament"), nil];
    for(NSInteger i=1; i<[config[@"blind_levels"] count]; i++) {
        [names addObject:[NSString stringWithFormat:NSLocalizedString(@"Round %zd", @"Numbered blind level"), i]];
    }
    return names;
}

#pragma mark Tournament Commands

// send a command through TournamentConnection
- (void)sendCommand:(NSString*)cmd withData:(NSDictionary*)arg andBlock:(void(^)(id))block {
    // add extra stuff to each command
    NSMutableDictionary* json = [[NSMutableDictionary alloc] init];

    // if commands already passed in
    if(arg != nil) {
        [json setDictionary:arg];
    }

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
    [self sendCommand:@"check_authorized" withData:nil andBlock:^(id json) {
        // set internal state if changed
        if([self authorized] != [json[@"authorized"] boolValue]) {
            [self setAuthorized:[json[@"authorized"] boolValue]];
        }
        // handle authorization check
        if(block) {
            block([self authorized]);
        }
    }];
}

- (void)getStateWithBlock:(void(^)(id))block {
    [self sendCommand:@"get_state" withData:nil andBlock:^(id json) {
        // handle config response
        if(block) {
            block(json);
        }
    }];
}


- (void)getConfigWithBlock:(void(^)(id))block {
    [self sendCommand:@"get_config" withData:nil andBlock:^(id json) {
        // handle config response
        if(block) {
            block(json);
        }
    }];
}

- (void)configure:(id)config withBlock:(void(^)(id))block {
    [self sendCommand:@"configure" withData:config andBlock:^(id json) {
        if(block) {
            block(json);
        }
    }];
}

- (void)resetState {
    [self sendCommand:@"reset_state" withData:nil andBlock:nil];
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
    [self sendCommand:@"set_previous_level" withData:nil andBlock:^(id json) {
        // handle blind level change
        if(block) {
            block(json[@"blind_level_changed"]);
        }
    }];
}

- (void)setNextLevelWithBlock:(void(^)(NSNumber*))block {
    [self sendCommand:@"set_next_level" withData:nil andBlock:^(id json) {
        // handle blind level change
        if(block) {
            block(json[@"blind_level_changed"]);
        }
    }];
}

- (void)setActionClock:(NSNumber*)milliseconds {
    [self sendCommand:@"set_action_clock" withData:@{@"duration" : milliseconds} andBlock:nil];
}

- (void)clearActionClock {
    [self sendCommand:@"set_action_clock" withData:nil andBlock:nil];
}

- (void)genBlindLevelsRequest:(NSDictionary*)request withBlock:(void(^)(NSArray*))block {
    [self sendCommand:@"gen_blind_levels" withData:request andBlock:^(id json) {
        // handle generated levels
        if(block) {
            block(json[@"blind_levels"]);
        }
    }];
}

- (void)fundPlayer:(id)playerId withFunding:(NSNumber*)sourceId {
    [self sendCommand:@"fund_player" withData:@{@"player_id" : playerId, @"source_id" : sourceId} andBlock:nil];
}

- (void)planSeatingFor:(NSNumber*)expectedPlayers withBlock:(void(^)(NSArray*))block {
    [self sendCommand:@"plan_seating" withData:@{@"max_expected_players" : expectedPlayers} andBlock:^(id json) {
        // handle player movement
        if(block) {
            block(json[@"players_moved"]);
        }
    }];
}

- (void)seatPlayer:(id)playerId withBlock:(void(^)(id,NSString*,NSString*,BOOL))block {
    [self sendCommand:@"seat_player" withData:@{@"player_id" : playerId} andBlock:^(id json) {
        // handle seated player
        id playerSeated = json[@"player_seated"];
        if(playerSeated) {
            if(block) {
                block(playerSeated[@"player_id"], playerSeated[@"table_name"], playerSeated[@"seat_name"], NO);
            }
        } else {
            id alreadySeated = json[@"already_seated"];
            if(alreadySeated) {
                if(block) {
                    block(alreadySeated[@"player_id"], alreadySeated[@"table_name"], alreadySeated[@"seat_name"], YES);
                }
            } else {
                if(block) {
                    block(nil, nil, nil, NO);
                }
            }
        }
    }];
}

- (void)unseatPlayer:(id)playerId {
    [self sendCommand:@"unseat_player" withData:@{@"player_id" : playerId} andBlock:nil];
}

- (void)bustPlayer:(id)playerId withBlock:(void(^)(NSArray*))block {
    [self sendCommand:@"bust_player" withData:@{@"player_id" : playerId} andBlock:^(id json) {
        // handle player movement
        if(block) {
            block(json[@"players_moved"]);
        }
    }];
}

- (void)rebalanceSeatingWithBlock:(void(^)(NSArray*))block {
    [self sendCommand:@"rebalance_seating" withData:nil andBlock:^(id json) {
        // handle player movement
        if(block) {
            block(json[@"players_moved"]);
        }
    }];
}

- (void)quickSetupWithBlock:(void(^)(NSArray*))block {
    [self sendCommand:@"quick_setup" withData:nil andBlock:^(id json) {
        // handle seated players
        if(block) {
            block(json[@"seated_players"]);
        }
    }];
}

#pragma mark Serialization

- (NSData*)dataWithResultsAsCSV {
    // header contains column names
    NSMutableString* csv = [[NSMutableString alloc] initWithString:NSLocalizedString(@"Player,Finish,Win", nil)];
    // 1 result per line
    [[self state][@"results"] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL* stop) {
        [csv appendString:[NSString stringWithFormat:@"\n\"%@\",%@,%@", obj[@"name"], obj[@"place"], obj[@"payout"]]];
    }];
    // serialize results to NSData. use WINDOWS-1252 for now
    return [csv dataUsingEncoding:NSWindowsCP1252StringEncoding];
}

#pragma mark Tournament Messages

- (void)handleMessage:(NSMutableDictionary*)json fromConnection:(TournamentConnection*)tc {
    // look for an error, pass it to the delegate
    NSString* errorString = json[@"error"];
    if(errorString) {
        // TODO: actually localize error string
        NSString* localizedDescription = errorString;
        NSError* error = [NSError errorWithDomain:TBErrorDomain code:TBErrorDaemonResponse userInfo:@{NSLocalizedDescriptionKey:localizedDescription}];
        if([[self delegate] respondsToSelector:@selector(tournamentSession:error:)]) {
            [[self delegate] tournamentSession:self error:error];
        }
    }

    // look for command key
    NSNumber* cmdkey = json[@"echo"];
    if(cmdkey != nil) {
        // remove command key response
        [json removeObjectForKey:@"echo"];

        // look up block for command key
        void (^block)(id) = [self blocksForCommands][cmdkey];
        if(block != nil) {
            if(errorString == nil) {
                // if it's a command with a handler block, and there is no error, call that block
                block(json);
            }

            // remove block from our dictionary
            [[self blocksForCommands] removeObjectForKey:cmdkey];
        }
    } else {
        // replace only state that changed
        NSDictionary* update = [json dictionaryWithChangesFromDictionary:[self state]];
        if([update count] > 0) {
            [[self state] addEntriesFromDictionary:update];
        }

        // erase state that is now missing
        NSSet* missing = [json missingKeysPresentInDictionary:[self state]];
        if([missing count] > 0) {
            [[self state] removeObjectsForKeys:[missing allObjects]];
        }
    }

    // special handling for the clocks because we may need to add an offset to what the daemon provides
    TBClockDateComponentsFormatter* dateFormatter = [[TBClockDateComponentsFormatter alloc] init];
    if([self state][@"clock_remaining"] && [self state][@"current_time"]) {
        // format time for display
        [self state][@"clock_text"] = [dateFormatter stringFromMillisecondsRemaining:[self state][@"clock_remaining"]
                                                             atMillisecondsSince1970:[self state][@"current_time"]
                                                                        countingDown:YES];
    }

    if([self state][@"elapsed_time"] && [self state][@"current_time"]) {
        [self state][@"elapsed_time_text"] = [dateFormatter stringFromMillisecondsRemaining:[self state][@"elapsed_time"]
                                                                    atMillisecondsSince1970:[self state][@"current_time"]
                                                                               countingDown:NO];
    }

    if(![[self state][@"running"] boolValue]) {
        [self state][@"clock_text"] = NSLocalizedString(@"PAUSED", nil);
    }
}

#pragma mark TournamentConnectionDelegate

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc {
    NSAssert([self connection] == tc, @"Unexpected connection from %@", tc);
    // successfully connected to tournament

    // clear state
    [[self state] removeAllObjects];

    // set connected state
    [self setConnected:YES];

    // always check if we're authorized right away
    [self checkAuthorizedWithBlock:nil];

    // and request initial state
    [self getStateWithBlock:^(id json) {
        [[self state] setDictionary:json];
        }];

    // notify delegate
    if([[self delegate] respondsToSelector:@selector(tournamentSessionDidBegin:)]) {
        [[self delegate] tournamentSessionDidBegin:self];
    }
}

- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc {
    NSAssert([self connection] == tc, @"Unexpected disconnection from %@", tc);
    // tournament disconnected

    [self disconnect];
}

- (void)tournamentConnectionDidClose:(TournamentConnection*)tc {
    NSAssert([self connection] == tc, @"Unexpected close from %@", tc);
    // close down connection (happens whether client or server disconnected)

    // clear state
    [[self state] removeAllObjects];

    // set disconnected
    [self setConnected:NO];
    [self setAuthorized:NO];

    // notify delegate
    if([[self delegate] respondsToSelector:@selector(tournamentSessionDidEnd:)]) {
        [[self delegate] tournamentSessionDidEnd:self];
    }
}

- (void)tournamentConnection:(TournamentConnection*)tc didReceiveData:(id)json {
    NSAssert([self connection] == tc, @"Unexpected data from %@", tc);
    [self handleMessage:json fromConnection:tc];
}

- (void)tournamentConnection:(TournamentConnection*)tc error:(NSError*)error {
    NSAssert([self connection] == tc, @"Unexpected error from %@", tc);

    // retry
    NSError* reconnectError;
    [[self connection] close];
    if([[self connection] connectToTournamentService:[self currentTournamentService] error:&reconnectError] == NO) {
        // notify delegate
        if([[self delegate] respondsToSelector:@selector(tournamentSession:error:)]) {
            [[self delegate] tournamentSession:self error:error];
        }

        [self disconnect];
    }
}

@end
