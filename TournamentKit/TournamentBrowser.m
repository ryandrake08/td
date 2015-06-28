//
//  TournamentBrowser.m
//  td
//
//  Created by Ryan Drake on 6/27/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentBrowser.h"
#import "TournamentService.h"

@interface TournamentBrowser () <NSNetServiceBrowserDelegate>

// service browser
@property (nonatomic, strong) NSNetServiceBrowser* serviceBrowser;

// list of known services
@property (nonatomic, strong) NSMutableArray* mutableServiceList;

@end

@implementation TournamentBrowser

- (instancetype)initWithDelegate:(id<TournamentBrowserDelegate>)delegate {
    if (self = [super init]) {
        // create service list
        _mutableServiceList = [[NSMutableArray alloc] init];

        // set delegate
        _delegate = delegate;

        // initialize service browser
        _serviceBrowser = [[NSNetServiceBrowser alloc] init];
        [[self serviceBrowser] setDelegate:self];
        [[self serviceBrowser] searchForServicesOfType:kTournamentServiceType inDomain:kTournamentServiceDomain];
    }
    return self;
}

- (void)dealloc {
    [[self serviceBrowser] stop];
    [[self serviceBrowser] setDelegate:nil];
}

- (NSArray*)localServiceList {
    // first check for unix sockets
    NSArray* directoryContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:NSTemporaryDirectory() error:nil];
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"SELF LIKE *tournamentd.*.sock"];
    return [directoryContents filteredArrayUsingPredicate:predicate];
}

- (NSArray*)remoteServiceList {
    return [self mutableServiceList];
}

#pragma mark NSNetServiceDelegate

- (void)netServiceBrowser:(NSNetServiceBrowser*)serviceBrowser didFindService:(NSNetService*)service moreComing:(BOOL)moreComing {
    [[self mutableServiceList] addObject:service];
    if(!moreComing) {
        [[self delegate] tournamentBrowser:self didUpdateRemoteServices:[self mutableServiceList]];
    }
}

- (void)netServiceBrowser:(NSNetServiceBrowser*)serviceBrowser didRemoveService:(NSNetService*)service moreComing:(BOOL)moreComing {
    [[self mutableServiceList] removeObject:service];
    if(!moreComing) {
        [[self delegate] tournamentBrowser:self didUpdateRemoteServices:[self mutableServiceList]];
    }
}

@end
