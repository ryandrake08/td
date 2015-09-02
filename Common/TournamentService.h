#pragma once

#define kTournamentServiceType @"_tournbuddy._tcp."
#define kTournamentServiceDomain @"local."
#define kTournamentServiceDefaultPort 25600

#import "TournamentConnection.h"

@interface TournamentService : NSObject

// initialize with either a NetService, address/port, or unix socket
- (instancetype)initWithNetService:(NSNetService*)service;
- (instancetype)initWithAddress:(NSString*)address andPort:(NSInteger)port;
- (instancetype)initWithUnixSocket:(NSString*)path;

// is this a remote service?
@property (nonatomic, readonly, getter=isRemote) BOOL remote;

// service name
@property (nonatomic, copy, readonly) NSString* name;

// retrieve the netService, if applicable
@property (nonatomic, strong, readonly) NSNetService* netService;

@end

@interface TournamentConnection (TournamentService)

// Category on TournamentConnection, to not foul up that class with TournamentSession
- (BOOL)connectToTournamentService:(TournamentService*)service;

@end