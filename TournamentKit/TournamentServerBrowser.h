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

@property (nonatomic, readonly, retain) NSMutableArray* serverList;

- (void)addServer:(TournamentServer*)server;

@end
