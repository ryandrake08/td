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

@interface Document () <NSTabViewDelegate>

// UI
@property IBOutlet NSTabView* tabView;
@property NSMutableDictionary* viewControllers;

// Model
@property TournamentDaemon* server;
@property TournamentSession* session;
@property NSMutableDictionary* configuration;

@end

@implementation Document

- (instancetype)init {
    self = [super init];
    if (self) {
        _server = [[TournamentDaemon alloc] init];
        _session = [[TournamentSession alloc] init];
        _viewControllers = [[NSMutableDictionary alloc] init];

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
    // Add any code here that needs to be executed once the windowController has loaded the document's window.

    // set tab view delegate
    [[self tabView] setDelegate:self];

    // get view controller for the tab selected in IB
    NSTabViewItem* selectedItem = [[self tabView] selectedTabViewItem];
    [self tabView:[self tabView] shouldSelectTabViewItem:selectedItem];
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
    [[self session] configure:[self configuration] withBlock:nil];
}

- (NSData*)dataOfType:(NSString*)typeName error:(NSError**)outError {
    // serialize json configuration to NSData
    return [NSJSONSerialization dataWithJSONObject:[self configuration] options:0 error:outError];
}

- (BOOL)readFromData:(NSData*)data ofType:(NSString*)typeName error:(NSError**)outError {
    // view controllers will need to be re-built (with different configuration object)
    [[self viewControllers] removeAllObjects];

    // deserialize json configuration
    [self setConfiguration:[NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:outError]];
    return *outError == nil;
}

#pragma mark Tabs

- (BOOL)tabView:(NSTabView*)tabView shouldSelectTabViewItem:(NSTabViewItem*)tabViewItem {
    // these tab view items' identifiers are keys and also identify their class
    NSString* nib = [tabViewItem identifier];

    NSViewController* controller = [self viewControllers][nib];
    if(controller == nil) {
        // create a clontroller for this view
        NSString* className = [nib stringByAppendingString:@"Controller"];
        controller = [[NSClassFromString(className) alloc] initWithNibName:nib configuration:[self configuration]];
        if(controller == nil) {
            // report error to user here
            NSLog(@"Can't load view for tab %@", nib);
            return NO;
        }

        // cache in dictionary for later
        [self viewControllers][nib] = controller;
    }

    // configure session when switching tabs (for now)
    [self configureSession];

    // set the view
    [tabViewItem setView:controller.view];
    return YES;
}

@end
