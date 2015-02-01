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

@interface Document () <NSTabViewDelegate>

// UI
@property IBOutlet NSTabView* tabView;
@property NSViewController* currentViewController;

// Model
@property TournamentDaemon* server;
@property TournamentSession* session;

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

#pragma mark Tabs

- (BOOL)tabView:(NSTabView*)tabView shouldSelectTabViewItem:(NSTabViewItem*)tabViewItem {
    NSViewController* newController = nil;

    // assume a different identifier has been assigned to each tab view item in IB
    NSInteger itemIndex = [tabView indexOfTabViewItemWithIdentifier:[tabViewItem identifier]];
    switch (itemIndex) {
        case 0:
            newController = [[TBFundingViewController alloc] initWithNibName:@"TBFundingView" bundle:nil];
            break;
        case 1:
            newController = [[TBPlayersViewController alloc] initWithNibName:@"TBPlayersView" bundle:nil];
            break;
        case 2:
            newController = [[TBChipsViewController alloc] initWithNibName:@"TBChipsView" bundle:nil];
            break;
        case 3:
            newController = [[TBRoundsViewController alloc] initWithNibName:@"TBRoundsView" bundle:nil];
            break;
        case 4:
            newController = [[TBSeatingViewController alloc] initWithNibName:@"TBSeatingView" bundle:nil];
            break;
    }

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
