//
//  TournamentServerBrowser.m
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentServerBrowser.h"

@interface TournamentServerBrowser()
@property (nonatomic, strong) NSMutableArray* serverList;
@end

@implementation TournamentServerBrowser

- (instancetype)init {
    if((self = [super init])) {
        _serverList = [NSMutableArray array];
    }

    return self;
}

- (void)addServer:(TournamentServerInfo*)server {
    [server setManuallyAdded:YES];
    [[self serverList] addObject:server];
}

- (NSUInteger)indexForServer:(TournamentServerInfo*)server {
    return [[self serverList] indexOfObject:server];
}

// get the server at given index
- (TournamentServerInfo*)serverForIndex:(NSUInteger)index {
    return [self serverList][index];
}

// get the number of servers
- (NSUInteger)serverCount {
    return [[self serverList] count];
}

@end
