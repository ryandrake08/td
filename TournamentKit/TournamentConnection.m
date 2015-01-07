//
//  TournamentConnection.m
//  TournamentKit
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentConnection.h"
#import "NSData+Delimiter.h"

#include <sys/socket.h>
#include <sys/un.h>

@interface TournamentConnection()
{
    // input and output streams
    NSInputStream* inputStream;
    NSOutputStream* outputStream;

    // YES if reported connection to delegate
    BOOL connected;

    // input/output buffers
    NSMutableData* inputBuffer;
    NSMutableData* outputBuffer;
}

@property (nonatomic, retain) TournamentServer* server;

@end

@implementation TournamentConnection
@synthesize delegate;
@dynamic connected;
@synthesize server;

- (id)initWithReadStream:(CFReadStreamRef)readStream writeStream:(CFWriteStreamRef)writeStream
{
    if((self = [super init])) {
        // initially unconnected
        connected = NO;

        // initialize buffers
        inputBuffer = [[NSMutableData alloc] init];
        outputBuffer = [[NSMutableData alloc] init];

        // specify that the CFStream should close itself when it's done
        (void) CFWriteStreamSetProperty(writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        (void) CFReadStreamSetProperty(readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);

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

- (id)initWithServer:(TournamentServer *)theServer {
    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
    CFStreamCreatePairWithSocketToHost(NULL, (CFStringRef)theServer.address, (UInt32)theServer.port, &readStream, &writeStream);

    self.server = theServer;

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

    self.server = nil;

    return [self initWithReadStream:readStream writeStream:writeStream];
}

- (void)dealloc {
    [self close];
    [super dealloc];
}

- (void)close {
    // close and shut down streams
    [inputStream close];
    [outputStream close];
    [inputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [outputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [inputStream release];
    [outputStream release];
    inputStream = nil;
    outputStream = nil;

    // no longer need buffers
    [inputBuffer release];
    [outputBuffer release];

    // mark no longer connected
    connected = NO;

    // notify delegate
    [delegate tournamentConnectionDidClose:self];
}

- (void)flushOutputBuffer {
    if([outputBuffer length] > 0) {
        NSInteger bytesWritten = [outputStream write:[outputBuffer bytes] maxLength:[outputBuffer length]];

        // remove written bytes
        if(bytesWritten > 0) {
            [outputBuffer replaceBytesInRange:NSMakeRange(0, bytesWritten) withBytes:NULL length:0];
        }

        // check error condition
        if(bytesWritten < 0) {
            // notify delegate
            [delegate tournamentConnection:self error:[outputStream streamError]];
        }
    }
}

- (void)fillInputBuffer {
    while([inputStream hasBytesAvailable]) {
        // read a chunk at a time
        uint8_t bytes[4096];
        NSInteger bytesRead = [inputStream read:bytes maxLength:4096];

        if(bytesRead < 0) {
            // notify delegate
            [delegate tournamentConnection:self error:[outputStream streamError]];
            return;
        }

        if(bytesRead > 0) {
            // append to inputBuffer
            [inputBuffer appendBytes:bytes length:bytesRead];
        }
    }
}

- (void)consumeInputBuffer {
    // search for newline delimiter
    NSRange subRange = [inputBuffer rangeOfDataDelimitedBy:'\n'];

    while(subRange.length > 0) {
        NSData* subBuffer = [inputBuffer subdataWithRange:subRange];

        // convert to JSON
        NSError* jsonError = nil;
        id jsonObject = [NSJSONSerialization JSONObjectWithData:subBuffer options:0 error:&jsonError];
        if(jsonError) {
            [delegate tournamentConnection:self error:jsonError];
        } else {
            [delegate tournamentConnection:self didReceiveData:jsonObject];
        }

        // empty that range
        [inputBuffer replaceBytesInRange:subRange withBytes:NULL length:0];

        // search again through reduced buffer
        subRange = [inputBuffer rangeOfDataDelimitedBy:'\n'];
    }
}

- (BOOL)sendCommand:(NSString*)cmd {
    // append command to output buffer
    [outputBuffer appendBytes:[cmd UTF8String] length:[cmd lengthOfBytesUsingEncoding:NSUTF8StringEncoding]];
    [outputBuffer appendBytes:"\n" length:sizeof("\n")];

    // send, if space available
    if([outputStream hasSpaceAvailable]) {
        [self flushOutputBuffer];
    }

    return YES;
}

- (BOOL)sendCommand:(NSString*)cmd withData:(id)jsonObject {
    // check object
    if(![NSJSONSerialization isValidJSONObject:jsonObject]) {
        return NO;
    }

    // convert cmd to data
    NSData* cmdData = [cmd dataUsingEncoding:NSUTF8StringEncoding];

    // convert json to data
    NSError* jsonError = nil;
    NSData* jsonData = [NSJSONSerialization dataWithJSONObject:jsonObject options:0 error:&jsonError];
    if(jsonData == nil) {
        // notify delegate
        [delegate tournamentConnection:self error:jsonError];
        return NO;
    }

    // append command to output buffer
    [outputBuffer appendData:cmdData];
    [outputBuffer appendBytes:" " length:1];
    [outputBuffer appendData:jsonData];
    [outputBuffer appendBytes:"\n" length:1];

    // send, if space available
    if([outputStream hasSpaceAvailable]) {
        [self flushOutputBuffer];
    }

    return YES;
}

- (BOOL)connected {
    NSStreamStatus inputStatus = [inputStream streamStatus];
    NSStreamStatus outputStatus = [outputStream streamStatus];

    BOOL inputConnected = inputStatus == NSStreamStatusOpen || inputStatus == NSStreamStatusReading || inputStatus == NSStreamStatusAtEnd;
    BOOL outputConnected = outputStatus == NSStreamStatusOpen || outputStatus == NSStreamStatusWriting || outputStatus == NSStreamStatusAtEnd;

    return inputConnected && outputConnected;
}

- (void)stream:(NSStream*)theStream handleEvent:(NSStreamEvent)streamEvent
{
    switch (streamEvent)
    {
        case NSStreamEventOpenCompleted:
            if(connected == NO && [self connected]) {
                connected = YES;
                [delegate tournamentConnectionDidConnect:self];
            }
            break;

        case NSStreamEventHasBytesAvailable:
            if(theStream == inputStream) {
                // fill up, then consume the input buffer
                [self fillInputBuffer];
                [self consumeInputBuffer];
            }
            break;

        case NSStreamEventHasSpaceAvailable:
            if(theStream == outputStream) {
                // flush any remaining data in the output buffer
                [self flushOutputBuffer];
            }
            break;

        case NSStreamEventErrorOccurred:
            // notify delegate
            [delegate tournamentConnection:self error:[theStream streamError]];
            break;

        case NSStreamEventEndEncountered:
            // notify delegate
            [delegate tournamentConnectionDidDisconnect:self];
            break;

        default:
            NSLog(@"Unknown event: %lu", streamEvent);
    }
}

@end
