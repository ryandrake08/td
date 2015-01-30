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

#define kDefaultTournamentListenPort 25600

@interface TournamentDaemon ()
{
    // flag to control whether or not daemon is running
    BOOL running;

    // semaphore to wait on indicating daemon is fully stopped
    dispatch_semaphore_t semaphore;

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
        semaphore = dispatch_semaphore_create(0);
        netService = nil;
        port = 0;
    }

    return self;
}

// start the daemon, pre-authorizing given client code, returning local unix socket path
- (NSString*)startWithAuthCode:(NSNumber*)code {

    // set up tournament and authorize
    __block tournament tourney;
    tourney.authorize([code intValue]);

    // start at default port, and increment until we find one that binds
    int try_service(kDefaultTournamentListenPort);
    bool trying(true);
    while(trying)
    {
        // build unique unix socket name using service name
        std::ostringstream local_server, inet_service;
        local_server << "/tmp/tournamentd." << try_service << ".sock";
        inet_service << try_service;

        try
        {
            // try to listen to this service
            tourney.listen(local_server.str().c_str(), inet_service.str().c_str());
            trying = false;
        }
        catch(const std::system_error& e)
        {
            // EADDRINUSE: failed to bind, probably another server on this port
            if(e.code().value() == EADDRINUSE)
            {
                try_service++;
                continue;
            }

            // re-throw anything not
            throw;
        }
    }

    // server is listening. mark as running and run in background
    running = YES;
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
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

        dispatch_semaphore_signal(semaphore);
    });

    // return the unix socket path, for subsequent local connection
    return [NSString stringWithFormat:@"/tmp/tournamentd.%d.sock", try_service];
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
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
}

@end
