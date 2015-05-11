//
//  Document.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "Document.h"
#import "TournamentKit/TournamentKit.h"
#import "TBChipsViewController.h"
#import "TBFundingViewController.h"
#import "TBPlayersViewController.h"
#import "TBRoundsViewController.h"
#import "TBSeatingViewController.h"

#import "NSObject+FBKVOController.h"

@interface Document () <NSTabViewDelegate>

// UI
@property IBOutlet NSTabView* tabView;
@property NSViewController* currentViewController;

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

        [[self KVOController] observe:self keyPath:@"configuration" options:0 action:@selector(syncWithSession)];

        // Start the session, connecting locally
        [[self session] connectToLocalPath:path];
    }
    return self;
}

- (void)close {
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

- (void)syncWithSession {
    [[self session] configure:[self configuration] withBlock:^(id json) {
        _configuration = json;
    }];
}

- (NSData*)dataOfType:(NSString*)typeName error:(NSError**)outError {
    [self syncWithSession];
    return [NSJSONSerialization dataWithJSONObject:[self configuration] options:0 error:outError];
}

- (BOOL)readFromData:(NSData*)data ofType:(NSString*)typeName error:(NSError**)outError {
    [self setConfiguration:[NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:outError]];
    return YES;
}

#pragma mark Tabs

- (BOOL)tabView:(NSTabView*)tabView shouldSelectTabViewItem:(NSTabViewItem*)tabViewItem {
    // assume a different identifier has been assigned to each tab view item in IB
    NSInteger itemIndex = [tabView indexOfTabViewItemWithIdentifier:[tabViewItem identifier]];

    // could be considered both clever and janky
    NSArray* tableNibs = @[@"TBFundingView", @"TBPlayersView", @"TBChipsView", @"TBRoundsView", @"TBSeatingView"];
    NSString* nib = [tableNibs objectAtIndex:itemIndex];
    NSString* className = [nib stringByAppendingString:@"Controller"];
    NSViewController* newController = [[NSClassFromString(className) alloc] initWithNibName:nib configuration:[self configuration]];

    if (newController != nil) {
        [tabViewItem setView:newController.view];
        // newController.view.frame = tabView.contentRect;
        [self setCurrentViewController:newController];
        return YES;
    } else {
        // report error to user here
        NSLog(@"Can't load view for tab %ld", (long)itemIndex);
        return NO;
    }
}

@end
