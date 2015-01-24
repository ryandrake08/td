//
//  TournamentDaemon.mm
//  TournamentKit
//
//  Created by Ryan Drake on 1/4/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentKit.h"
#import "TournamentDaemon.h"

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

- (void)startWithService:(NSString*)service authCode:(int)code {
    NSString* name = [NSString stringWithFormat:kDefaultTournamentLocalPath, service];

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        running = YES;

        tournament tourney;
        tourney.authorize(code);
        tourney.listen([name cStringUsingEncoding:NSUTF8StringEncoding], [service cStringUsingEncoding:NSUTF8StringEncoding]);

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
