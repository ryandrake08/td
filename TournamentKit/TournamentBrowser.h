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

- (instancetype)init;

// start searching for tournaments
- (void)search;

// list of known services (TournamentService* )
- (NSArray*)serviceList;
- (NSArray*)localServiceList;
- (NSArray*)remoteServiceList;

// delegate
@property (nonatomic, weak) IBOutlet id <TournamentBrowserDelegate> delegate;

@end

@protocol TournamentBrowserDelegate <NSObject>

// called after a new list of remote services has been received
- (void)tournamentBrowser:(TournamentBrowser*)tournamentBroswer didUpdateServices:(NSArray*)services;

@end
