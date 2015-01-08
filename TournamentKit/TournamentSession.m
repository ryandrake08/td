//
//  TournamentSession.m
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentSession.h"
#import "TournamentConnection.h"

#define kDefaultTournamentLocalPath @"/tmp/tournamentd.sock"

@interface TournamentSession() <TournamentConnectionDelegate>

@property (nonatomic, strong) TournamentConnection* connection;
@property (nonatomic, assign) BOOL authorized;
@property (nonatomic, strong) NSMutableDictionary* blocksForCommands;

@end

@implementation TournamentSession

@synthesize connectionDelegate;
@dynamic currentServer;
@synthesize authorized;
@synthesize connection;
@synthesize blocksForCommands;

- (void)connectToLocal {
    // if we're connected remotely, disconnect
    if([[self connection] server]) {
        [self setConnection:nil];
    }

    // at this point, if connection is not nil, we're already connected locally
    if([self connection] == nil) {
        [self setConnection:[[TournamentConnection alloc] initWithUnixSocketNamed:kDefaultTournamentLocalPath]];
        [[self connection] setDelegate:self];
    }
}

- (void)connectToServer:(TournamentServerInfo*)theServer {
    if([[self connection] server] != theServer) {
        [self setConnection:[[TournamentConnection alloc] initWithServer:theServer]];
        [[self connection] setDelegate:self];
    }
}

- (void)disconnect {
    [self setConnection:nil];
}

- (TournamentServerInfo*)currentServer {
    return [[self connection] server];
}

#pragma mark Internal routines

// client identifier (used for authenticating with servers)
+ (NSNumber*)clientIdentifier {
#if defined(DEBUG)
    return @31337;
#else
    NSString* key = @"clientIdentifier";
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    // if we don't already have one
    if([defaults objectForKey:key] == nil) {
        // generate a new identifier
        u_int32_t cid = arc4random() % 90000 + 10000;
        [defaults setObject:[NSNumber numberWithInteger:cid] forKey:key];
    }
    return [defaults objectForKey:key];
#endif
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
        // TODO: Handle error
        // handle authorization check
        [self setAuthorized:[json[@"authorized"] boolValue]];
        block([json[@"authorized"] boolValue]);
    }];
}

- (void)authorize:(NSNumber*)clientId withBlock:(void(^)(NSNumber*))block {
    [self sendCommand:@"authorize" withData:@{@"authorize" : clientId} andBlock:^(id json, NSString* error) {
        // TODO: Handle error
        // handle client authorization
        block(json[@"authorized_client"]);
    }];
}

- (void)startGameAt:(NSDate*)datetime {
    if(datetime) {
        [self sendCommand:@"start_game" withData:@{@"start_at" : datetime} andBlock:nil];
    } else {
        [self sendCommand:@"start_game" withData:nil andBlock:nil];
    }
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

- (void)setPreviousLevelWithBlock:(void(^)(NSNumber*))block {
    [self sendCommand:@"set_previous_level" withData:nil andBlock:^(id json, NSString* error) {
        // TODO: Handle error
        // handle blind level change
        block(json[@"blind_level_changed"]);
    }];
}

- (void)setNextLevelWithBlock:(void(^)(NSNumber*))block {
    [self sendCommand:@"set_next_level" withData:nil andBlock:^(id json, NSString* error) {
        // TODO: Handle error
        // handle blind level change
        block(json[@"blind_level_changed"]);
    }];
}

- (void)setActonClock:(NSNumber*)milliseconds {
    if(milliseconds) {
        [self sendCommand:@"set_action_clock" withData:@{@"duration" : milliseconds} andBlock:nil];
    } else {
        [self sendCommand:@"set_action_clock" withData:nil andBlock:nil];
    }
}

- (void)genBlindLevelsCount:(NSNumber*)count withDuration:(NSNumber*)milliseconds {
    [self sendCommand:@"gen_blind_levels" withData:@{@"duration" : milliseconds, @"count" : count} andBlock:nil];
}

- (void)resetFunding {
    [self sendCommand:@"reset_funding" withData:nil andBlock:nil];
}

- (void)fundPlayer:(NSNumber*)playerId withFunding:(NSNumber*)sourceId {
    [self sendCommand:@"fund_player" withData:@{@"player" : playerId, @"source_id" : sourceId} andBlock:nil];
}

- (void)planSeatingFor:(NSNumber*)expectedPlayers {
    [self sendCommand:@"plan_seating" withData:@{@"max_expected_players" : expectedPlayers} andBlock:nil];
}

- (void)seatPlayer:(NSNumber*)playerId withBlock:(void(^)(NSNumber*,NSNumber*,NSNumber*))block {
    [self sendCommand:@"seat_player" withData:@{@"player" : playerId} andBlock:^(id json, NSString* error) {
        // TODO: Handle error
        // handle seated player
        id playerSeated = json[@"player_seated"];
        if(playerSeated) {
            block(playerSeated[@"player_id"], playerSeated[@"table_number"], playerSeated[@"seat_number"]);
        } else {
            block(nil, nil, nil);
        }
    }];
}

- (void)bustPlayer:(NSNumber*)playerId withBlock:(void(^)(NSArray*))block {
    [self sendCommand:@"bust_player" withData:@{@"player" : playerId} andBlock:^(id json, NSString* error) {
        // TODO: Handle error
        // handle player movement
        // for now, just hand back the json
        // TODO: make this more sophisticated and populate a separate NSArray with objects
        block(json[@"players_moved"]);
    }];
}

#pragma mark Tournament Messages

- (void)handleMessage:(id)json fromConnection:(TournamentConnection*)tc {
    // look for command key
    NSNumber* cmdkey = json[@"echo"];
    if(cmdkey) {
        // look up block for command key
        void (^block)(id,NSString*) = [self blocksForCommands][cmdkey];
        if(block) {
            // if it's a command with a handler block, call that block
            block(json, json[@"error"]);

            // remove it from our dictionary
            [[self blocksForCommands] removeObjectForKey:cmdkey];
        }
    } else {
        // TODO: handle non-command
    }
}

#pragma mark TournamentConnectionDelegate

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidConnect");
    NSAssert([self connection] == tc, @"Unexpected connection from %@", tc);
    if([tc server]) {
        [[self connectionDelegate] tournamentSession:self connectionStatusDidChange:[tc server] connected:YES];
    }
}

- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidDisconnect");
    NSAssert([self connection] == tc, @"Unexpected disconnection from %@", tc);
    [self setConnection:nil];
    [self setAuthorized:NO];
}

- (void)tournamentConnectionDidClose:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidClose");
    NSAssert([self connection] == nil, @"Connection %@ closed while session retains", tc);
    if([tc server]) {
        [[self connectionDelegate] tournamentSession:self connectionStatusDidChange:[tc server] connected:NO];
    }
    [self setConnection:nil];
    [self setAuthorized:NO];
}

- (void)tournamentConnection:(TournamentConnection*)tc didReceiveData:(id)json {
    NSLog(@"+++ tournamentConnectionDidReceiveData");
    NSAssert([self connection] == tc, @"Unexpected data from %@", tc);
    [self handleMessage:json fromConnection:tc];
}

- (void)tournamentConnection:(TournamentConnection*)tc error:(NSError*)error {
    NSLog(@"+++ tournamentConnectionError: %@", [error localizedDescription]);
    NSAssert([self connection] == tc, @"Unexpected error from %@", tc);
    [self setConnection:nil];
    [self setAuthorized:NO];
}

#pragma mark Singleton Methods

+ (instancetype)sharedSession {
    static TournamentSession* sharedMySession = nil;
    @synchronized(self) {
        if(sharedMySession == nil) {
            sharedMySession = [[self alloc] init];
        }
    }
    return sharedMySession;
}

- (instancetype)init {
    if (self = [super init]) {
        blocksForCommands = [[NSMutableDictionary alloc] init];
    }
    return self;
}

@end
