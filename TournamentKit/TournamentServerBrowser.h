//
//  TournamentServerBrowser.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TournamentServer.h"

@interface TournamentServerBrowser : NSObject

@property (nonatomic, readonly, strong) NSMutableArray* serverList;

// manually add a server to the list
- (void)addServer:(TournamentServer*)server;

// get an indexPath to a particular list item
- (NSUInteger)indexForServer:(TournamentServer*)server;

@end
