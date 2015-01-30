#include "CFStreamCreatePairWithUnixSocket.h"

#include <CFNetwork/CFSocketStream.h>

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

void CFStreamCreatePairWithUnixSocket(CFAllocatorRef alloc, CFStringRef name, CFReadStreamRef* readStream, CFWriteStreamRef* writeStream)
{
    struct sockaddr_un addr;

    // Check path length
    const char* path = CFStringGetCStringPtr(name, kCFStringEncodingUTF8);
    if(strlen(path) > sizeof(addr.sun_path)-1)
    {
        fprintf(stderr, "Socket name too long");
        return;
    }

    // Set up addr
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
    addr.sun_family = AF_UNIX;
    addr.sun_len = SUN_LEN(&addr);

    // Create socket
    int sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if(sock == -1)
    {
        fprintf(stderr, "Call to socket failed");
        return;
    }

    // Set SO_NOSIGPIPE option
    int yes = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)) == -1)
    {
        fprintf(stderr, "Call to setsockopt failed");
        close(sock);
        return;
    }

    if(connect(sock, (struct sockaddr*)&addr, addr.sun_len) == -1)
    {
        fprintf(stderr, "Call to connect failed");
        close(sock);
        return;
    }

    CFStreamCreatePairWithSocket(kCFAllocatorDefault, sock, readStream, writeStream);

    // specify that the CFStream should close itself when it's done
    if(readStream != NULL && *readStream != NULL)
    {
        (void) CFReadStreamSetProperty(*readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
    }
    if(writeStream != NULL && *writeStream != NULL)
    {
        (void) CFWriteStreamSetProperty(*writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
    }
}