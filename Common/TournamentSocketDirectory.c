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

// return directory suitable for our unix socket
const char* TournamentSocketDirectory(void) {
    struct sockaddr_un sun;
    size_t maxLength = sizeof(sun.sun_path) - strlen("tournamentd.12345.sock");

    static char tmpdir[PATH_MAX];
    size_t n = confstr(_CS_DARWIN_USER_TEMP_DIR, tmpdir, sizeof(tmpdir));
    if ((n <= 0) || (n >= sizeof(tmpdir)) || (n >= maxLength)) {
        n = strlcpy(tmpdir, getenv("TMPDIR"), sizeof(tmpdir));
        if(n >= maxLength) {
            strcpy(tmpdir, "/tmp");
        }
    }
    return tmpdir;
}
