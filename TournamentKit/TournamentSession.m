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

static TournamentSession *sharedMySession = nil;

@interface TournamentSession() <TournamentConnectionDelegate>

@property (nonatomic, retain) TournamentConnection* connection;
@property (nonatomic, assign) BOOL isAuthorized;

@end

@implementation TournamentSession

@synthesize connectionDelegate;
@synthesize connection;
@dynamic currentServer;
@synthesize isAuthorized;

- (void)connectToLocal {
    // if we're connected remotely, disconnect
    if(self.connection.server) {
        self.connection = nil;
    }

    // at this point, if connection is not nil, we're already connected locally
    if(self.connection == nil) {
        self.connection = [[[TournamentConnection alloc] initWithUnixSocketNamed:kDefaultTournamentLocalPath] autorelease];
        [self.connection setDelegate:self];
    }
}

- (void)connectToServer:(TournamentServer*)theServer {
    if(self.connection.server != theServer) {
        self.connection = [[[TournamentConnection alloc] initWithServer:theServer] autorelease];
        [self.connection setDelegate:self];
    }
}

- (void)disconnect {
    self.connection = nil;
}

- (TournamentServer*)currentServer {
    return self.connection.server;
}

#pragma mark Internal routines

// client identifier (used for authenticating with servers)
+ (NSNumber*)clientIdentifier {
#if defined(DEBUG)
    return [NSNumber numberWithInt:31337];
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
    return [NSNumber numberWithInt:incrementingKey++];
}

#pragma mark Tournament Commands

// send a command through TournamentConnection
- (void)sendCommand:(NSString*)cmd withData:(NSDictionary*)arg {
    // add extra stuff to each command
    NSMutableDictionary* json = [NSMutableDictionary dictionaryWithDictionary:arg];

    // append to every command: authentication
    NSNumber* cid = [TournamentSession clientIdentifier];
    [json setObject:cid forKey:@"authenticate"];

    // append to every command: command key
    NSNumber* cmdkey = [TournamentSession commandKey];
    [json setObject:cmdkey forKey:@"echo"];

    // send it through connection
    [[self connection] sendCommand:cmd withData:json];
}

- (void)checkAuthorized {
    [self sendCommand:@"check_authorized" withData:nil];
}

- (void)authorize:(NSNumber*)clientId {
    [self sendCommand:@"authorize" withData:@{@"authorize" : clientId}];
}

- (void)startGameAt:(NSDate*)datetime {
    if(datetime) {
        [self sendCommand:@"start_game" withData:@{@"start_at" : datetime}];
    } else {
        [self sendCommand:@"start_game" withData:nil];
    }
}

- (void)stopGame {
    [self sendCommand:@"stop_game" withData:nil];
}

- (void)resumeGame {
    [self sendCommand:@"resume_game" withData:nil];
}

- (void)pauseGame {
    [self sendCommand:@"pause_game" withData:nil];
}

- (void)setPreviousLevel {
    [self sendCommand:@"set_previous_level" withData:nil];
}

- (void)setNextLevel {
    [self sendCommand:@"set_next_level" withData:nil];
}

- (void)setActonClock:(NSNumber*)milliseconds {
    if(milliseconds) {
        [self sendCommand:@"set_action_clock" withData:@{@"duration" : milliseconds}];
    } else {
        [self sendCommand:@"set_action_clock" withData:nil];
    }
}

- (void)genBlindLevelsCount:(NSNumber*)count withDuration:(NSNumber*)milliseconds {
    [self sendCommand:@"gen_blind_levels" withData:@{@"duration" : milliseconds, @"count" : count}];
}

- (void)resetFunding {
    [self sendCommand:@"reset_funding" withData:nil];
}

- (void)fundPlayer:(NSNumber*)playerId withFunding:(NSNumber*)sourceId {
    [self sendCommand:@"fund_player" withData:@{@"player" : playerId, @"source_id" : sourceId}];
}

- (void)planSeatingFor:(NSNumber*)expectedPlayers {
    [self sendCommand:@"plan_seating" withData:@{@"max_expected_players" : expectedPlayers}];
}

- (void)seatPlayer:(NSNumber*)playerId {
    [self sendCommand:@"seat_player" withData:@{@"player" : playerId}];
}

- (void)bustPlayer:(NSNumber*)playerId {
    [self sendCommand:@"bust_player" withData:@{@"player" : playerId}];
}

#pragma mark Tournament Messages

- (void)handleMessage:(id)json fromConnection:(TournamentConnection*)tc {
    // handle error
    id error = [json objectForKey:@"error"];
    if(error) {
        NSLog(@"Error from server: %@", error);
        return;
    }

    // look for command key
    NSNumber* cmdkey = [json objectForKey:@"echo"];
    if(cmdkey) {
        // do nothing with for now
    }

    // handle authorization check
    id authorized = [json objectForKey:@"authorized"];
    if(authorized) {
        self.isAuthorized = authorized;
        [connectionDelegate tournamentSession:self authorizationStatusDidChange:tc.server authorized:[authorized boolValue]];
    }

    // handle client authorization
    id authorizedClient = [json objectForKey:@"authorized_client"];
    if(authorizedClient) {
        NSLog(@"+++ authorized client: %@", authorizedClient);
    }

    // handle blind level change
    id blindLevelChanged = [json objectForKey:@"blind_level_changed"];
    if(blindLevelChanged) {
        NSLog(@"+++ blind level changed: %@", blindLevelChanged);
    }

    // handle seated player
    id playerSeated = [json objectForKey:@"player_seated"];
    if(playerSeated) {
        NSLog(@"+++ player seated: %@", playerSeated);
    }

    // handle player movement
    id playersMoved = [json objectForKey:@"players_moved"];
    if(playersMoved) {
        NSLog(@"+++ players moved: %@", playersMoved);
    }
}

#pragma mark TournamentConnectionDelegate

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidConnect");
    NSAssert(self.connection == tc, @"Unexpected connection from %@", tc);
    if(tc.server) {
        [connectionDelegate tournamentSession:self connectionStatusDidChange:tc.server connected:YES];
    }
}

- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidDisconnect");
    NSAssert(self.connection == tc, @"Unexpected disconnection from %@", tc);
    self.connection = nil;
    self.isAuthorized = NO;
}

- (void)tournamentConnectionDidClose:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidClose");
    NSAssert(self.connection == nil, @"Connection %@ closed while session retains", tc);
    if(tc.server) {
        [connectionDelegate tournamentSession:self connectionStatusDidChange:tc.server connected:NO];
    }
    self.connection = nil;
    self.isAuthorized = NO;
}

- (void)tournamentConnection:(TournamentConnection*)tc didReceiveData:(id)json {
    NSLog(@"+++ tournamentConnectionDidReceiveData");
    NSAssert(self.connection == tc, @"Unexpected data from %@", tc);
    [self handleMessage:json fromConnection:tc];
}

- (void)tournamentConnection:(TournamentConnection*)tc error:(NSError*)error {
    NSLog(@"+++ tournamentConnectionError: %@", [error localizedDescription]);
    NSAssert(self.connection == tc, @"Unexpected error from %@", tc);
    self.connection = nil;
    self.isAuthorized = NO;
}

#pragma mark Singleton Methods

+ (id)sharedSession {
    @synchronized(self) {
        if(sharedMySession == nil)
            sharedMySession = [[super allocWithZone:NULL] init];
    }
    return sharedMySession;
}
+ (id)allocWithZone:(NSZone *)zone {
    return [[self sharedSession] retain];
}
- (id)copyWithZone:(NSZone *)zone {
    return self;
}
- (id)retain {
    return self;
}
- (NSUInteger)retainCount {
    return UINT_MAX; //denotes an object that cannot be released
}
- (oneway void)release {
    // never release
}
- (id)autorelease {
    return self;
}
- (id)init {
    if (self = [super init]) {
    }
    return self;
}
- (void)dealloc {
    [connection release];
    [super dealloc];
}

@end
