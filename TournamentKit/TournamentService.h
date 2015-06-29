#pragma once

#define kTournamentServiceType @"_tournbuddy._tcp."
#define kTournamentServiceDomain @"local."

#import "TournamentConnection.h"

@interface TournamentService : NSObject

// initialize with either a NetService, address/port, or unix socket
- (instancetype)initWithNetService:(NSNetService*)service;
- (instancetype)initWithAddress:(NSString*)address andPort:(NSInteger)port;
- (instancetype)initWithUnixSocket:(NSString*)path;

// connect to a service
- (BOOL)connectTo:(TournamentConnection*)connection;

// is this a remote service?
- (BOOL)isRemote;

// service name
- (NSString*)name;

// retrieve the netService, if applicable
- (NSNetService*)netService;

@end