//
//  Document.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "Document.h"
#import "TBConfigurationWindowController.h"
#import "TBSeatingViewController.h"
#import "TournamentKit/TournamentKit.h"
#import "NSObject+FBKVOController.h"

@interface Document ()

// UI
@property (strong) IBOutlet TBSeatingViewController* viewController;
@property (strong) IBOutlet TBConfigurationWindowController* configurationWindowController;

// Model
@property (strong) TournamentDaemon* server;
@property (strong) TournamentSession* session;
@property (strong) NSMutableDictionary* configuration;

@end

@implementation Document

- (instancetype)init {
    self = [super init];
    if (self) {
        _server = [[TournamentDaemon alloc] init];
        _session = [[TournamentSession alloc] init];
        _configuration = [[NSMutableDictionary alloc] init];

        // register for KVO
        [[self KVOController] observe:[self session] keyPath:@"isConnected" options:0 block:^(id observer, id object, NSDictionary *change) {
            if([object isConnected]) {
                // check authorization
                [object checkAuthorizedWithBlock:^(BOOL authorized) {
                    NSLog(@"Connected and authorized locally");
                }];
            }
        }];

        // Start serving using this device's auth key
        NSString* path = [[self server] startWithAuthCode:[TournamentSession clientIdentifier]];

        // Start the session, connecting locally
        [[self session] connectToLocalPath:path];
    }
    return self;
}

- (void)close {
    [[self configurationWindowController] close];
    [[self KVOController] unobserveAll];
    [[self session] disconnect];
    [[self server] stop];
    [super close];
}

- (void)windowControllerDidLoadNib:(NSWindowController*)aController {
    [super windowControllerDidLoadNib:aController];

    // setup main view
    [[self viewController] setConfiguration:[self configuration]];
    [[self viewController] setSession:[self session]];
    [[aController window] setContentView:[[self viewController] view]];

    // setup configuration view
    TBConfigurationWindowController* configurationWindow = [[TBConfigurationWindowController alloc] initWithWindowNibName:@"TBConfigurationWindow"];
    [self setConfigurationWindowController:configurationWindow];
    [[self configurationWindowController] setSession:[self session]];
    [[self configurationWindowController] setConfiguration:[self configuration]];
    [[self configurationWindowController] showWindow:self];
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

    // send to session
    [[self session] configure:[self configuration] withBlock:^(id json) {
        if(![json isEqual:[self configuration]]) {
            [[self configuration] setDictionary:json];
        }
    }];

    return *outError == nil;
}

@end
