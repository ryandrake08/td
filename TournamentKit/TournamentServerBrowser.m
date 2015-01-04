//
//  TournamentServerBrowser.m
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentServerBrowser.h"

@implementation TournamentServerBrowser

@synthesize serverList;

- (id)init {
    if((self = [super init])) {
        serverList = [[NSMutableArray alloc] init];
    }

    return self;
}

- (void)dealloc {
    [serverList release];
    [super dealloc];
}

- (void)addServer:(TournamentServer*)server {
    server.manuallyAdded = YES;
    [serverList addObject:server];
}

- (NSUInteger)indexForServer:(TournamentServer*)server {
    return [serverList indexOfObject:server];
}

@end
