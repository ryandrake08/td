#include "CFStreamCreatePairWithUnixSocket.h"

#include <CFNetwork/CFSocketStream.h>
#include <CoreFoundation/CFNumber.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

void CFStreamCreatePairWithUnixSocket(CFAllocatorRef alloc, CFStringRef name, CFReadStreamRef* readStream, CFWriteStreamRef* writeStream)
{
    // Clear outputs
    if(readStream) {
        *readStream = NULL;
    }
    if(writeStream) {
        *writeStream = NULL;
    }

    // Get C-style string (the unix socket path name) from name
    char buffer[FILENAME_MAX];
    const char* path = CFStringGetCStringPtr(name, kCFStringEncodingUTF8);
    if(path == NULL) {
        if(!CFStringGetCString(name, buffer, FILENAME_MAX, kCFStringEncodingUTF8)) {
            fprintf(stderr, "Filename buffer not large enough\n");
            return;
        }
        path = buffer;
    }

    // Check path length
    struct sockaddr_un addr;
    if(strlen(path) > sizeof(addr.sun_path)-1)
    {
        fprintf(stderr, "Socket name too long: %s\n", path);
        return;
    }

    // Set up addr
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
    addr.sun_family = AF_UNIX;
    addr.sun_len = (unsigned char)SUN_LEN(&addr);

    // Create socket
    int sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if(sock == -1)
    {
        fprintf(stderr, "Call to socket failed: %s\n", strerror(errno));
        return;
    }

    // Set SO_NOSIGPIPE option
    int yes = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)) == -1)
    {
        fprintf(stderr, "Call to setsockopt failed: %s\n", strerror(errno));
        close(sock);
        return;
    }

    // Connect socket to addr
    if(connect(sock, (struct sockaddr*)&addr, addr.sun_len) == -1)
    {
        fprintf(stderr, "Call to connect failed: %s\n", strerror(errno));
        close(sock);
        return;
    }

    // Create the socket pair
    CFStreamCreatePairWithSocket(alloc, sock, readStream, writeStream);

    // Specify that the CFStream should close itself when it's released
    if(readStream != NULL && *readStream != NULL)
    {
        (void) CFReadStreamSetProperty(*readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
    }
    if(writeStream != NULL && *writeStream != NULL)
    {
        (void) CFWriteStreamSetProperty(*writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
    }
}
