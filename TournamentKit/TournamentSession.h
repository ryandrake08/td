//
//  TournamentSession.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TournamentServer.h"

#define kDefaultTournamentServerPort 25600

@protocol TournamentSessionConnectionDelegate;

@interface TournamentSession : NSObject

@property (nonatomic, assign) id<TournamentSessionConnectionDelegate> connectionDelegate;
@property (nonatomic, readonly, retain) TournamentServer* currentServer;

- (void)connectToLocal;
- (void)connectToServer:(TournamentServer*)server;

// Singleton instance
+ (id)sharedSession;

@end

@protocol TournamentSessionConnectionDelegate <NSObject>

- (void)tournamentSession:(TournamentSession*)session didConnectToServer:(TournamentServer*)server;
- (void)tournamentSession:(TournamentSession*)session didDisconnectFromServer:(TournamentServer*)server;

@end