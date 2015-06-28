//
//  TournamentBrowser.h
//  td
//
//  Created by Ryan Drake on 6/27/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol TournamentBrowserDelegate;

@interface TournamentBrowser : NSObject

- (instancetype)initWithDelegate:(id <TournamentBrowserDelegate>)delegate;

// list of known unix sockets (NSString* path)
- (NSArray*)localServiceList;

// list of known services (NSNetService*)
- (NSArray*)remoteServiceList;

// delegate
@property (nonatomic, weak) id <TournamentBrowserDelegate> delegate;

@end

@protocol TournamentBrowserDelegate <NSObject>

// called after a new list of remote services has been received
- (void)tournamentBrowser:(TournamentBrowser*)tournamentBroswer didUpdateRemoteServices:(NSArray*)services;

@end
