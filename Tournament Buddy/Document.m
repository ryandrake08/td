//
//  Document.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "Document.h"
#import "TournamentKit/TournamentKit.h"
#import "TBTableViewController.h"
#import "NSObject+FBKVOController.h"
#import "NSString+CamelCase.h"

@interface Document () <NSTabViewDelegate>

// UI
@property (strong) IBOutlet NSTabView* tabView;

// Controllers
@property (strong) IBOutlet TBTableViewController* chipsViewController;
@property (strong) IBOutlet TBTableViewController* fundingViewController;
@property (strong) IBOutlet TBTableViewController* playersViewController;
@property (strong) IBOutlet TBTableViewController* roundsViewController;
@property (strong) IBOutlet TBTableViewController* seatingViewController;

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

        // Start serving using this device's auth key
        NSString* path = [[self server] startWithAuthCode:[TournamentSession clientIdentifier]];

        // register for KVO
        [[self KVOController] observe:[self session] keyPath:@"isConnected" options:0 block:^(id observer, id object, NSDictionary *change) {
            if([object isConnected]) {
                // check authorization
                [object checkAuthorizedWithBlock:^(BOOL authorized) {
                    NSLog(@"Connected and authorized locally");
                }];
            }
        }];

        // pass whole-configuration changes to session
        [[self KVOController] observe:self keyPath:@"configuration" options:0 action:@selector(configureSession)];

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

    // get view controller for the tab selected in IB
    NSTabViewItem* selectedItem = [[self tabView] selectedTabViewItem];
    [self tabView:[self tabView] didSelectTabViewItem:selectedItem];
}

+ (BOOL)autosavesInPlace {
    return YES;
}

- (NSString*)windowNibName {
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"Document";
}

- (void)configureSession {
    // only send parts of configuration that changed
    NSMutableDictionary* configToSend = [[self configuration] mutableCopy];
    NSMutableArray* keysToRemove = [NSMutableArray array];
    [configToSend enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL* stop) {
        NSString* propertyName = [key asCamelCaseFromUnderscore];
        if([obj isEqual:[[self session] valueForKey:propertyName]]) {
            [keysToRemove addObject:key];
        }
    }];
    [configToSend removeObjectsForKeys:keysToRemove];

    if([configToSend count] > 0) {
        [[self session] configure:configToSend withBlock:^(id json) {
            if(![json isEqual:[self configuration]]) {
                [[self configuration] setDictionary:json];
            }
        }];
    }
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

#pragma mark Tabs

- (void)tabView:(NSTabView*)tabView didSelectTabViewItem:(NSTabViewItem*)tabViewItem {
    // these tab view items' identifiers correspond cleverly to their instance variables
    TBTableViewController* controller = [self valueForKey:[tabViewItem identifier]];

    // set configuration and session
    if([controller configuration] == nil) {
        [controller setConfiguration: [self configuration]];
    }
    if([controller session] == nil) {
        [controller setSession:[self session]];
    }

    // configure session when switching tabs (for now)
    [self configureSession];

    // set the view
    [tabViewItem setView:controller.view];
}

@end
