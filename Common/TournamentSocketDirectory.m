//
//  TournamentSocketDirectory.m
//  td
//
//  Created by Ryan Drake on 9/16/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#include "TournamentSocketDirectory.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

#import <Foundation/Foundation.h>

// return directory suitable for our unix socket
const char* TournamentSocketDirectory(void) {
    // get maximum size of a unix socket path
    struct sockaddr_un sun;
    size_t maxlength = sizeof(sun.sun_path) - strlen("/tournamentd.12345.sock");

    // static string holding actual path returned
    static char tmpdir[PATH_MAX];
    size_t n = 0;

    // first try sandbox-friendly path (Cocoa Foundation)
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    if(paths != nil) {
        NSString* path = [paths firstObject];
        if(path != nil) {
            const char* cpath = [path UTF8String];
            if(cpath != 0) {
                n = strlcpy(tmpdir, cpath, sizeof(tmpdir));
                if((n > 0) && (n <= maxlength)) {
                    return tmpdir;
                }
            }
        }
    }

    // no sandbox-friendly path, try darwin user temp directory
    n = confstr(_CS_DARWIN_USER_TEMP_DIR, tmpdir, sizeof(tmpdir));
    if((n > 0) && (n <= sizeof(tmpdir)) && (n <= maxlength)) {
        return tmpdir;
    }

    // darwin user temp directory too long, try TMPDIR environment
    n = strlcpy(tmpdir, getenv("TMPDIR"), sizeof(tmpdir));
    if((n > 0) && (n <= maxlength)) {
        return tmpdir;
    }

    // TMPDIR environment too long, use /tmp
    return "/tmp";
}
