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

@dynamic server;
@synthesize connection;

- (void)connectLocally {
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

- (TournamentServer*)server {
    return self.connection.server;
}

#pragma mark TournamentConnectionDelegate

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidConnect");
}

- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidDisconnect");
}

- (void)tournamentConnectionDidClose:(TournamentConnection*)tc {
    NSLog(@"+++ tournamentConnectionDidClose");
    self.connection = nil;
}

- (void)tournamentConnection:(TournamentConnection*)tc didReceiveData:(id)json {
    NSLog(@"+++ tournamentConnectionDidReceiveData: %@", json);
}

- (void)tournamentConnection:(TournamentConnection*)tc error:(NSError*)error {
    NSLog(@"+++ tournamentConnectionError: %@", [error localizedDescription]);
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
