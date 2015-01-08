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

- (instancetype)init {
    if((self = [super init])) {
        serverList = [[NSMutableArray alloc] init];
    }

    return self;
}

- (void)addServer:(TournamentServerInfo*)server {
    [server setManuallyAdded: YES];
    [[self serverList] addObject:server];
}

- (NSUInteger)indexForServer:(TournamentServerInfo*)server {
    return [[self serverList] indexOfObject:server];
}

@end
