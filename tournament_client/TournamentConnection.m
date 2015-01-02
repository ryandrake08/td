//
//  TournamentConnection.m
//  tournament_client
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentConnection.h"

@interface TournamentConnection()
{
    NSInputStream* inputStream;
    NSOutputStream* outputStream;
}
@end

@implementation TournamentConnection

- (id)initWithHostname:(NSString*)hostname port:(UInt32)port {

    if((self = [super init])) {
        CFReadStreamRef readStream;
        CFWriteStreamRef writeStream;
        CFStreamCreatePairWithSocketToHost(NULL, (CFStringRef)hostname, port, &readStream, &writeStream);

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

- (void)dealloc {

    [inputStream release];

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
