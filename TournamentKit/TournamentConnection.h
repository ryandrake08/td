//
//  TournamentConnection.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TournamentServer.h"

@protocol TournamentConnectionDelegate;

@interface TournamentConnection : NSObject <NSStreamDelegate>

- (id)initWithUnixSocketNamed:(NSString*)socketPath;
- (id)initWithServer:(TournamentServer*)server;
- (BOOL)sendCommand:(NSString*)cmd;
- (BOOL)sendCommand:(NSString*)cmd withData:(id)jsonObject;
- (void)close;

@property (nonatomic, assign) id <TournamentConnectionDelegate> delegate;
@property (nonatomic, readonly, assign) BOOL connected;
@property (nonatomic, readonly, retain) TournamentServer* server;

@end

@protocol TournamentConnectionDelegate <NSObject>

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc;
- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc;
- (void)tournamentConnectionDidClose:(TournamentConnection*)tc;
- (void)tournamentConnection:(TournamentConnection*)tc didReceiveData:(id)json;
- (void)tournamentConnection:(TournamentConnection*)tc error:(NSError*)error;

@end
