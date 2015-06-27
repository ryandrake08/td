//
//  TournamentDaemon.mm
//  TournamentKit
//
//  Created by Ryan Drake on 1/4/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentDaemon.h"

#include "tournament.hpp"
#include <sstream>

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

    // place unix sockets in temp directory
    std::string tmppath = [NSTemporaryDirectory() UTF8String];

    // set up tournament and authorize
    __block tournament tourney;
    tourney.authorize([code intValue]);
    auto service(tourney.listen(tmppath));

    // server is listening. mark as running and run in background
    running = YES;
    dispatch_group_async(group, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        while(running)
        {
            try
            {
                auto quit = tourney.run();
                running = running && !quit;
            }
            catch(const std::system_error& e)
            {
                // EINTR: select() was interrupted. Just retry
                if(e.code().value() != EINTR)
                {
                    throw;
                }
            }
        }
    });

    // store the port
    port = service.second;

    // return the unix socket path, for subsequent local connection
    return [NSString stringWithUTF8String:service.first.c_str()];
}

// publish over Bojour using name
- (void)publishWithName:(NSString*)name {
    // stop NetService
    [netService stop];

    if(name && port) {
        // set up and publish NetService
        netService = [[NSNetService alloc] initWithDomain:@"local." type:@"_tournbuddy._tcp." name:name port:port];
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
