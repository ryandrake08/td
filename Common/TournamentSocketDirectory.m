//
//  TournamentSocketDirectory.m
//  td
//
//  Created by Ryan Drake on 9/16/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TournamentSocketDirectory.h"

@import Darwin.POSIX.sys.un;

// return directory suitable for our unix socket
NSString* TournamentSocketDirectory(void) {
    struct sockaddr_un sun;
    size_t maxLength = sizeof(sun.sun_path) - strlen("tournamentd.12345.sock");

    // use NSTemporaryDirectory if it's not too long
    NSString* tmp = NSTemporaryDirectory();
    if([tmp length] > maxLength) {
        tmp = @"/tmp";
    }
    return tmp;
}
