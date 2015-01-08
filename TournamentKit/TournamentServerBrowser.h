//
//  TournamentServerBrowser.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TournamentServerInfo.h"

@interface TournamentServerBrowser : NSObject

@property (nonatomic, readonly, assign) NSUInteger serverCount;

// manually add a server to the list
- (void)addServer:(TournamentServerInfo*)server;

// get an index to a particular list item
- (NSUInteger)indexForServer:(TournamentServerInfo*)server;

// get the server at given index
- (TournamentServerInfo*)serverForIndex:(NSUInteger)index;

@end
