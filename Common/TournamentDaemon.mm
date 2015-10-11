//
//  TournamentDaemon.mm
//  td
//
//  Created by Ryan Drake on 1/4/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentDaemon.h"
#import "TournamentService.h"
#import "TournamentSocketDirectory.h"

#include "tournament.hpp"
#include "logger.hpp"

@interface TournamentDaemon ()
{
    // flag to control whether or not daemon is running
    BOOL running;

    // dispatch group to wait on indicating daemon is fully stopped
    dispatch_group_t group;

    // listening port
    int port;

    // published Bonjour service
    NSNetService* netService;
}

@end

@implementation TournamentDaemon

- (instancetype)init {
    if((self = [super init])) {
        running = NO;
        group = dispatch_group_create();
        netService = nil;
        port = 0;
    }

    return self;
}

// start the daemon, pre-authorizing given client code, returning local unix socket path
- (NSString*)startWithAuthCode:(NSNumber*)code {

    logger_enable(LOG_ERROR, LOG_WARNING);

    // set up tournament and authorize
    __block tournament tourney;
    tourney.authorize([code intValue]);
    auto service(tourney.listen(TournamentSocketDirectory()));

    // server is listening. mark as running and run in background
    running = YES;
    dispatch_group_async(group, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        while(running)
        {
            auto quit = tourney.run();
            running = running && !quit;
        }
    });

    // store the port
    port = service.second;

    // return the unix socket path, for subsequent local connection
    return @(service.first.c_str());
}

// publish over Bojour using name
- (void)publishWithName:(NSString*)name {
    // stop NetService
    [netService stop];

    if(name && port) {
        // set up and publish NetService
        netService = [[NSNetService alloc] initWithDomain:kTournamentServiceDomain type:kTournamentServiceType name:name port:port];
        [netService publish];
    }
}

// stop the daemon (and stop publishing if doing so)
- (void)stop {
    // stop NetService
    [netService stop];

    // clear port
    port = 0;

    // signal block execution to stop
    running = NO;

    // wait for a block execution
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
}

@end
