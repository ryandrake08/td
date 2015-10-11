//
//  TournamentBrowser.m
//  td
//
//  Created by Ryan Drake on 6/27/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentBrowser.h"
#import "TournamentService.h"
#import "TournamentSocketDirectory.h"

@interface TournamentBrowser () <NSNetServiceBrowserDelegate>

// service browser
@property (nonatomic, strong) NSNetServiceBrowser* serviceBrowser;

// list of known tournaments
@property (nonatomic, strong) NSMutableArray* mutableList;

@end

@implementation TournamentBrowser

- (instancetype)init {
    if (self = [super init]) {
        // create lists
        _mutableList = [[NSMutableArray alloc] init];

        // initialize service browser
        _serviceBrowser = [[NSNetServiceBrowser alloc] init];
        [[self serviceBrowser] setDelegate:self];

        // socket path
        NSString* tmpPath = [NSString stringWithUTF8String:TournamentSocketDirectory()];

        // get list of unix sockets
        NSArray* directoryContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:tmpPath error:nil];
        NSPredicate* predicate = [NSPredicate predicateWithFormat:@"SELF LIKE '*tournamentd.*.sock'"];
        NSArray* tournamentContents = [directoryContents filteredArrayUsingPredicate:predicate];

        // add a tournament for each
        for(NSString* filename in tournamentContents) {
            NSString* fullPath = [tmpPath stringByAppendingPathComponent:filename];
            TournamentService* tournament = [[TournamentService alloc] initWithUnixSocket:fullPath];
            [[self mutableList] addObject:tournament];
        }

    }
    return self;
}

- (void)dealloc {
    [[self serviceBrowser] stop];
    [[self serviceBrowser] setDelegate:nil];
}

- (void)search {
    if([self delegate] == nil) {
        NSLog(@"TournamentBrowser searching with no delegate");
    }
    [[self serviceBrowser] searchForServicesOfType:kTournamentServiceType inDomain:kTournamentServiceDomain];
}

- (NSArray*)serviceList {
    return [self mutableList];
}

- (NSArray*)localServiceList {
    NSPredicate* localOnly = [NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary *bindings) {
        return ![evaluatedObject isRemote];
    }];
    return [[self mutableList] filteredArrayUsingPredicate:localOnly];
}

- (NSArray*)remoteServiceList {
    NSPredicate* remoteOnly = [NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary *bindings) {
        return [evaluatedObject isRemote];
    }];
    return [[self mutableList] filteredArrayUsingPredicate:remoteOnly];
}

#pragma mark NSNetServiceDelegate

- (void)netServiceBrowser:(NSNetServiceBrowser*)serviceBrowser didFindService:(NSNetService*)service moreComing:(BOOL)moreComing {
    // add a tournament
    TournamentService* tournament = [[TournamentService alloc] initWithNetService:service];
    [[self mutableList] addObject:tournament];

    // notify delegate
    if(!moreComing) {
        [[self delegate] tournamentBrowser:self didUpdateServices:[self mutableList]];
    }
}

- (void)netServiceBrowser:(NSNetServiceBrowser*)serviceBrowser didRemoveService:(NSNetService*)service moreComing:(BOOL)moreComing {
    for(TournamentService* tournament in [self mutableList]) {
        if([[tournament netService] isEqual:service]) {
            [[self mutableList] removeObject:tournament];
            break;
        }
    }

    // notify delegate
    if(!moreComing) {
        [[self delegate] tournamentBrowser:self didUpdateServices:[self mutableList]];
    }
}

@end
