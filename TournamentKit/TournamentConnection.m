//
//  TournamentConnection.m
//  TournamentKit
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentConnection.h"

#include <sys/socket.h>
#include <sys/un.h>

@interface TournamentConnection()
{
    NSInputStream* inputStream;
    NSOutputStream* outputStream;
}
@end

@implementation TournamentConnection

- (id)initWithReadStream:(CFReadStreamRef)readStream writeStream:(CFWriteStreamRef)writeStream
{
    if((self = [super init])) {
        // specify that the CFStream should close itself when it's done
        (void) CFReadStreamSetProperty(readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        (void) CFWriteStreamSetProperty(writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);

        // toll-free bridge the streams
        inputStream = (NSInputStream*)readStream;
        outputStream = (NSOutputStream*)writeStream;

        // set up the streams
        [inputStream setDelegate:self];
        [outputStream setDelegate:self];
        [inputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [outputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [inputStream open];
        [outputStream open];
    }

    return self;
}

- (id)initWithHostname:(NSString*)hostname port:(UInt32)port {

    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
    CFStreamCreatePairWithSocketToHost(NULL, (CFStringRef)hostname, port, &readStream, &writeStream);

    return [self initWithReadStream:readStream writeStream:writeStream];
}

- (id)initWithUnixSocketNamed:(NSString*)socketPath {

    struct sockaddr_un addr;

    // Check path length
    const char* path = [socketPath cStringUsingEncoding:NSUTF8StringEncoding];
    if(strlen(path) > sizeof(addr.sun_path)-1)
    {
        NSLog(@"Socket name too long");
        return nil;
    }

    // Set up addr
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
    addr.sun_family = AF_UNIX;
    addr.sun_len = SUN_LEN(&addr);

    // Unlink old socket path
    unlink(path);

    // Create socket
    int sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if(sock == -1)
    {
        NSLog(@"Call to socket failed");
        return nil;
    }

    // Set SO_NOSIGPIPE option
    int yes = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)) == -1)
    {
        NSLog(@"Call to setsockopt failed");
        close(sock);
        return nil;
    }

    if(connect(sock, (struct sockaddr*)&addr, addr.sun_len) == -1)
    {
        NSLog(@"Call to connect failed");
        close(sock);
        return nil;
    }

    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
    CFStreamCreatePairWithSocket (kCFAllocatorDefault, sock, &readStream, &writeStream);

    return [self initWithReadStream:readStream writeStream:writeStream];
}

- (void)dealloc {

    [inputStream release];
    [outputStream release];

    [super dealloc];
}

- (void)stream:(NSStream*)theStream handleEvent:(NSStreamEvent)streamEvent
{
    NSLog(@"Handle Event - ");
    switch (streamEvent)
    {
        case NSStreamEventOpenCompleted:
            NSLog(@"Stream opened");
            break;

        case NSStreamEventHasBytesAvailable:
            NSLog(@"Bytes Available!");
            if(theStream == inputStream)
            {
                NSLog(@"inputStream is ready.");

                uint8_t buf[1024];
                NSInteger len = [inputStream read:buf maxLength:1024];

                if(len > 0)
                {
                    NSMutableData* data=[[NSMutableData alloc] initWithLength:0];

                    [data appendBytes: (const void *)buf length:len];

                    NSString* string = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
                    NSLog(@"Server said- %@", string);

                    // TODO: Handle
                }
            }
            break;

        case NSStreamEventErrorOccurred:
            NSLog(@"Can not connect to the host!");
            break;

        case NSStreamEventEndEncountered:
            NSLog(@"End Encountered");
            break;

        case NSStreamEventHasSpaceAvailable:
            NSLog(@"Space Availible");
            break;

        default:
            NSLog(@"Unknown event: %lu", streamEvent);
    }
}

@end
