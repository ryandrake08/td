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

@end

@implementation TournamentSession

@synthesize connectionDelegate;
@synthesize connection;
@dynamic currentServer;

- (void)connectToLocal {
    // if we're connected remotely, disconnect
    if(self.connection.server != nil) {
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

#pragma mark Tournament Commands

- (void)checkAuthorized {
    NSNumber* cid = [TournamentSession clientIdentifier];
    NSDictionary* json = [NSDictionary dictionaryWithObject:cid forKey:@"authenticate"];
    [[self connection] sendCommand:@"check_authorized" withData:json];
}

#pragma mark Tournament Messages

- (void)handleMessage:(id)json fromConnection:(TournamentConnection*)tc {
    // handle authorization
    id authorized = [json objectForKey:@"authorized"];
    if(authorized) {
        tc.server.authorized = [authorized boolValue];
        [connectionDelegate tournamentSession:self authorizationStatusDidChange:tc.server authorized:tc.server.authorized];
    }

    id error = [json objectForKey:@"error"];
    if(error) {
        NSLog(@"Error from server: %@", error);
    }
}

#pragma mark TournamentConnectionDelegate

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidConnect");
    NSAssert(self.connection == tc, @"Unexpected connection from %@", tc);
    if(tc.server != nil) {
        [connectionDelegate tournamentSession:self connectionStatusDidChange:tc.server connected:YES];
    }

    // query authentication status if necessary
    if(tc.server == nil || tc.server.authenticate) {
        [self checkAuthorized];
    }
}

- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidDisconnect");
    NSAssert(self.connection == tc, @"Unexpected disconnection from %@", tc);
    self.connection = nil;
}

- (void)tournamentConnectionDidClose:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidClose");
    NSAssert(self.connection == nil, @"Connection %@ closed while session retains", tc);
    if(tc.server != nil) {
        [connectionDelegate tournamentSession:self connectionStatusDidChange:tc.server connected:NO];
    }
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
