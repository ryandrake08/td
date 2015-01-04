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

- (id)initWithUnixSocketNamed:(NSString*)socketPath;
- (id)initWithHostname:(NSString*)hostname port:(UInt32)port;
- (BOOL)sendCommand:(NSString*)cmd;
- (BOOL)sendCommand:(NSString*)cmd withData:(id)jsonObject;
- (void)close;

@property (assign) id <TournamentConnectionDelegate> delegate;
@property (readonly, assign) BOOL connected;

@end

@protocol TournamentConnectionDelegate <NSObject>

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc;
- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc;
- (void)tournamentConnectionDidClose:(TournamentConnection*)tc;
- (void)tournamentConnection:(TournamentConnection*)tc didReceiveData:(id)json;
- (void)tournamentConnection:(TournamentConnection*)tc error:(NSError*)error;

@end
