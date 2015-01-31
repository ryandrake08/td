//
//  Document.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "Document.h"
#import "TournamentKit/TournamentKit.h"

@interface Document ()

@property (strong) TournamentDaemon* server;
@property (strong) TournamentSession* session;

@end

@implementation Document

- (instancetype)init {
    self = [super init];
    if (self) {
        _server = [[TournamentDaemon alloc] init];
        _session = [[TournamentSession alloc] init];

        // Start serving using this device's auth key
        NSString* path = [[self server] startWithAuthCode:[TournamentSession clientIdentifier]];

        // register for KVO
        [[self session] addObserver:self forKeyPath:NSStringFromSelector(@selector(isConnected)) options:0 context:NULL];

        // Start the session, connecting locally
        [[self session] connectToLocalPath:path];
    }
    return self;
}

- (void)close {
    [[self session] removeObserver:self forKeyPath:NSStringFromSelector(@selector(isConnected))];
    [[self session] disconnect];
    [[self server] stop];
    [super close];
}

- (void)windowControllerDidLoadNib:(NSWindowController*)aController {
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
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
    id jsonObject = [[self session] currentConfiguration];
    return [NSJSONSerialization dataWithJSONObject:jsonObject options:0 error:outError];
}

- (BOOL)readFromData:(NSData*)data ofType:(NSString*)typeName error:(NSError**)outError {
    id jsonObject = [NSJSONSerialization JSONObjectWithData:data options:0 error:outError];
    [[self session] configure:jsonObject];
    return YES;
}

#pragma mark KVO

- (void)observeValueForKeyPath:(NSString*)keyPath ofObject:(id)session change:(NSDictionary*)change context:(void*)context {
    if([session isKindOfClass:[TournamentSession class]]) {
        if([keyPath isEqualToString:NSStringFromSelector(@selector(isConnected))]) {
            if([session isConnected]) {
                // check authorization
                [session checkAuthorizedWithBlock:^(BOOL authorized) {
                    NSLog(@"Connected and authorized locally");
                }];
            }
        }
    }
}

@end
