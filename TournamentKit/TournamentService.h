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
@property (nonatomic, readonly, getter=isRemote) BOOL remote;

// service name
@property (nonatomic, copy, readonly) NSString* name;

// retrieve the netService, if applicable
@property (nonatomic, strong, readonly) NSNetService* netService;

@end