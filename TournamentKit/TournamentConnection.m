//
//  TournamentConnection.m
//  TournamentKit
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentConnection.h"
#import "NSData+Delimiter.h"
#include "CFStreamCreatePairWithUnixSocket.h"

@interface TournamentConnection()

// input and output streams
@property (nonatomic, strong) NSInputStream* inputStream;
@property (nonatomic, strong) NSOutputStream* outputStream;

// input/output buffers
@property (nonatomic, strong) NSMutableData* inputBuffer;
@property (nonatomic, strong) NSMutableData* outputBuffer;

@end

@implementation TournamentConnection
@synthesize inputStream;
@synthesize outputStream;
@synthesize inputBuffer;
@synthesize outputBuffer;
@synthesize delegate;
@dynamic connected;

- (instancetype)init {
    if((self = [super init])) {
        // initialize buffers
        inputBuffer = [NSMutableData data];
        outputBuffer = [NSMutableData data];
    }

    return self;
}

- (void)setupReadStream:(CFReadStreamRef)readStream writeStream:(CFWriteStreamRef)writeStream
{
    // toll-free bridge the streams
    [self setInputStream:(__bridge_transfer NSInputStream*)readStream];
    [self setOutputStream:(__bridge_transfer NSOutputStream*)writeStream];

    // set up the streams
    [[self inputStream] setDelegate:self];
    [[self outputStream] setDelegate:self];
    [[self inputStream] scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [[self outputStream] scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [[self inputStream] open];
    [[self outputStream] open];
}

- (void)connectToAddress:(NSString*)address andPort:(NSInteger)port {
    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
    CFStreamCreatePairWithSocketToHost(kCFAllocatorDefault, (__bridge CFStringRef)address, (UInt32)port, &readStream, &writeStream);

    [self setupReadStream:readStream writeStream:writeStream];
}

- (void)connectToUnixSocketNamed:(NSString*)socketPath {
    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
    CFStreamCreatePairWithUnixSocket(kCFAllocatorDefault, (__bridge CFStringRef)socketPath, &readStream, &writeStream);

    [self setupReadStream:readStream writeStream:writeStream];
}

- (void)dealloc {
    [self close];
}

- (void)close {
    // no longer need contents of buffers
    [[self inputBuffer] setLength:0];
    [[self outputBuffer] setLength:0];

    // remove from run loop
    [[self inputStream] removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [[self outputStream] removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

    if([self isConnected]) {
        // close and shut down streams
        [[self inputStream] close];
        [[self outputStream] close];

        // notify delegate
        [[self delegate] tournamentConnectionDidClose:self];
    }

    // set to nil
    [self setInputStream:nil];
    [self setOutputStream:nil];
}

- (void)flushOutputBuffer {
    if([[self outputBuffer] length] > 0) {
        NSInteger bytesWritten = [[self outputStream] write:[[self outputBuffer] bytes] maxLength:[[self outputBuffer] length]];

        // remove written bytes
        if(bytesWritten > 0) {
            [[self outputBuffer] replaceBytesInRange:NSMakeRange(0, bytesWritten) withBytes:NULL length:0];
        }

        // check error condition
        if(bytesWritten < 0) {
            // notify delegate
            [[self delegate] tournamentConnection:self error:[[self outputStream] streamError]];
        }
    }
}

- (void)fillInputBuffer {
    while([[self inputStream] hasBytesAvailable]) {
        // read a chunk at a time
        uint8_t bytes[4096];
        NSInteger bytesRead = [[self inputStream] read:bytes maxLength:4096];

        if(bytesRead < 0) {
            // notify delegate
            [[self delegate] tournamentConnection:self error:[[self outputStream] streamError]];
            return;
        }

        if(bytesRead > 0) {
            // append to [self inputBuffer]
            [[self inputBuffer] appendBytes:bytes length:bytesRead];
        }
    }
}

- (void)consumeInputBuffer {
    // search for newline delimiter
    NSRange subRange = [[self inputBuffer] rangeOfDataDelimitedBy:'\n'];

    while(subRange.length > 0) {
        NSData* subBuffer = [[self inputBuffer] subdataWithRange:subRange];

        // convert to JSON
        NSError* jsonError = nil;
        id jsonObject = [NSJSONSerialization JSONObjectWithData:subBuffer options:0 error:&jsonError];
        if(jsonError) {
            [[self delegate] tournamentConnection:self error:jsonError];
        } else {
            [[self delegate] tournamentConnection:self didReceiveData:jsonObject];
        }

        // empty that range
        [[self inputBuffer] replaceBytesInRange:subRange withBytes:NULL length:0];

        // search again through reduced buffer
        subRange = [[self inputBuffer] rangeOfDataDelimitedBy:'\n'];
    }
}

- (BOOL)sendCommand:(NSString*)cmd {
    // append command to output buffer
    [[self outputBuffer] appendBytes:[cmd UTF8String] length:[cmd lengthOfBytesUsingEncoding:NSUTF8StringEncoding]];
    [[self outputBuffer] appendBytes:"\n" length:sizeof("\n")];

    // send, if space available
    if([[self outputStream] hasSpaceAvailable]) {
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
        [[self delegate] tournamentConnection:self error:jsonError];
        return NO;
    }

    // append command to output buffer
    [[self outputBuffer] appendData:cmdData];
    [[self outputBuffer] appendBytes:" " length:1];
    [[self outputBuffer] appendData:jsonData];
    [[self outputBuffer] appendBytes:"\n" length:1];

    // send, if space available
    if([[self outputStream] hasSpaceAvailable]) {
        [self flushOutputBuffer];
    }

    return YES;
}

- (BOOL)isConnected {
    NSStreamStatus inputStatus = [[self inputStream] streamStatus];
    NSStreamStatus outputStatus = [[self outputStream] streamStatus];

    BOOL inputConnected = inputStatus == NSStreamStatusOpen || inputStatus == NSStreamStatusReading || inputStatus == NSStreamStatusAtEnd;
    BOOL outputConnected = outputStatus == NSStreamStatusOpen || outputStatus == NSStreamStatusWriting || outputStatus == NSStreamStatusAtEnd;

    return inputConnected && outputConnected;
}

- (void)stream:(NSStream*)theStream handleEvent:(NSStreamEvent)streamEvent
{
    switch (streamEvent)
    {
        case NSStreamEventOpenCompleted:
            if(theStream == [self outputStream]) {
                // report connection state based on output stream connection
                // so we report connection only once
                [[self delegate] tournamentConnectionDidConnect:self];
            }
            break;

        case NSStreamEventHasBytesAvailable:
            if(theStream == [self inputStream]) {
                // fill up, then consume the input buffer
                [self fillInputBuffer];
                [self consumeInputBuffer];
            }
            break;

        case NSStreamEventHasSpaceAvailable:
            if(theStream == [self outputStream]) {
                // flush any remaining data in the output buffer
                [self flushOutputBuffer];
            }
            break;

        case NSStreamEventErrorOccurred:
            // notify delegate
            [[self delegate] tournamentConnection:self error:[theStream streamError]];
            break;

        case NSStreamEventEndEncountered:
            // notify delegate
            [[self delegate] tournamentConnectionDidDisconnect:self];
            break;

        default:
            NSLog(@"Unknown event: %lu", (unsigned long)streamEvent);
    }
}

@end
