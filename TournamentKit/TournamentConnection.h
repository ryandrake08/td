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

// open a connection to the server, either local or remote
- (id)initWithUnixSocketNamed:(NSString*)socketPath;
- (id)initWithServer:(TournamentServer*)server;

// send a text command to the server
- (BOOL)sendCommand:(NSString*)cmd;

// send a text command to the server with a JSON-encodable object as an argument
- (BOOL)sendCommand:(NSString*)cmd withData:(id)jsonObject;

// close the connection to the server
- (void)close;

// delegate to receive connect, disconnect, error and data messages
@property (nonatomic, assign) id <TournamentConnectionDelegate> delegate;

// returns true if currently connected to a server
@property (nonatomic, readonly, assign) BOOL connected;

// information about the connected remote server. nil if unconnected or connected locally
@property (nonatomic, readonly, retain) TournamentServer* server;

@end

@protocol TournamentConnectionDelegate <NSObject>

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc;
- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc;
- (void)tournamentConnectionDidClose:(TournamentConnection*)tc;
- (void)tournamentConnection:(TournamentConnection*)tc didReceiveData:(id)json;
- (void)tournamentConnection:(TournamentConnection*)tc error:(NSError*)error;

@end
