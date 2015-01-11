//
//  TournamentConnection.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol TournamentConnectionDelegate;

@interface TournamentConnection : NSObject <NSStreamDelegate>

// open a connection to the server, either local or remote
- (void)connectToUnixSocketNamed:(NSString*)socketPath;
- (void)connectToAddress:(NSString*)address andPort:(NSInteger)port;

// send a text command to the server
- (BOOL)sendCommand:(NSString*)cmd;

// send a text command to the server with a JSON-encodable object as an argument
- (BOOL)sendCommand:(NSString*)cmd withData:(id)jsonObject;

// close the connection to the server
- (void)close;

// delegate to receive connect, disconnect, error and data messages
@property (nonatomic, weak) id <TournamentConnectionDelegate> delegate;

// returns true if currently connected to a server
@property (nonatomic, readonly, assign, getter=isConnected) BOOL connected;

@end

@protocol TournamentConnectionDelegate <NSObject>

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc;
- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc;
- (void)tournamentConnectionDidClose:(TournamentConnection*)tc;
- (void)tournamentConnection:(TournamentConnection*)tc didReceiveData:(id)json;
- (void)tournamentConnection:(TournamentConnection*)tc error:(NSError*)error;

@end
