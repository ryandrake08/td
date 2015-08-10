//
//  Document.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "Document.h"
#import "TBSeatingViewController.h"
#import "TournamentKit/TournamentKit.h"
#import "NSObject+FBKVOController.h"

@interface Document ()

// Model
@property (strong) TournamentDaemon* server;
@property (strong) TournamentSession* session;
@property (strong) NSMutableDictionary* configuration;

// UI
@property (strong) TBSeatingViewController* viewController;

@end

@implementation Document

- (instancetype)init {
    self = [super init];
    if (self) {
        _server = [[TournamentDaemon alloc] init];
        _session = [[TournamentSession alloc] init];
        _configuration = [[NSMutableDictionary alloc] init];

        _viewController = [[TBSeatingViewController alloc] initWithNibName:@"TBSeatingView" bundle:nil];
        [_viewController setSession:_session];
        [_viewController setConfiguration:_configuration];

        // register for KVO
        [[self KVOController] observe:[self session] keyPath:@"isConnected" options:0 block:^(id observer, id object, NSDictionary *change) {
            if([object isConnected]) {
                // check authorization
                [object checkAuthorizedWithBlock:^(BOOL authorized) {
                    NSLog(@"Connected and authorized locally");

                    // on connection, send entire configuration to session, unconditionally, and then replace with whatever the session has
                    NSLog(@"Synchronizing session unconditionally");
                    [[self session] configure:[self configuration] withBlock:^(id json) {
                        if(![json isEqual:[self configuration]]) {
                            NSLog(@"Document differs from session");
                            [[self configuration] setDictionary:json];
                        }
                    }];
                }];
            }
        }];

        [[self KVOController] observe:[self session] keyPath:@"name" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
            [[self server] publishWithName:[object name]];
        }];

        // Start serving using this device's auth key
        NSString* path = [[self server] startWithAuthCode:[TournamentSession clientIdentifier]];

        // Start the session, connecting locally
        [[self session] connectToLocalPath:path];
    }
    return self;
}

- (void)close {
    [[self KVOController] unobserveAll];
    [[self session] disconnect];
    [[self server] stop];
    [super close];
}

- (void)windowControllerDidLoadNib:(NSWindowController*)aController {
    [super windowControllerDidLoadNib:aController];

    // show main view
    [[aController window] setContentView:[[self viewController] view]];
}

+ (BOOL)autosavesInPlace {
    return YES;
}

- (NSString*)windowNibName {
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"Document";
}

- (NSData*)dataOfType:(NSString*)typeName error:(NSError**)outError {
    // serialize json configuration to NSData
    return [NSJSONSerialization dataWithJSONObject:[self configuration] options:0 error:outError];
}

- (BOOL)readFromData:(NSData*)data ofType:(NSString*)typeName error:(NSError**)outError {
    // deserialize json configuration
    [[self configuration] setDictionary:[NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:outError]];
    return *outError == nil;
}

- (NSPrintOperation*)printOperationWithSettings:(NSDictionary*)ps error:(NSError**)outError {
    NSPrintInfo* printInfo = [self printInfo];
    NSPrintOperation* printOp = [NSPrintOperation printOperationWithView:[[self viewController] view] printInfo:printInfo];
    return printOp;
}

@end
