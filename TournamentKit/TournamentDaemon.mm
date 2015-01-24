//
//  TournamentDaemon.mm
//  TournamentKit
//
//  Created by Ryan Drake on 1/4/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentKit.h"
#import "TournamentDaemon.h"
#import "TournamentSession.h"

#include "tournament.hpp"

@interface TournamentDaemon ()
{
    BOOL running;
    dispatch_semaphore_t semaphore;
}
@end

@implementation TournamentDaemon

- (instancetype)init {
    if((self = [super init])) {
        running = NO;
        semaphore = dispatch_semaphore_create(0);
    }

    return self;
}

- (void)startWithService:(NSString*)service authCode:(NSNumber*)code {
    NSString* localServer = [TournamentSession localServerForService:service];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        // bring parameters over from objective-c
        auto auth_code = [code intValue];
        auto local_server = [localServer cStringUsingEncoding:NSUTF8StringEncoding];
        auto inet_service = [service cStringUsingEncoding:NSUTF8StringEncoding];

        running = YES;

        tournament tourney;
        tourney.authorize(auth_code);
        tourney.listen(local_server, inet_service);

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
}

- (void)stop {
    running = NO;
    // Wait for a block execution
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
}


@end
