//
//  Document.m
//  td
//
//  Created by Ryan Drake on 1/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "Document.h"
#import "TBSeatingViewController.h"
#import "TBResultsViewController.h"
#import "TBPlayersViewController.h"
#import "TBConfigurationWindowController.h"
#import "TBPlayerWindowController.h"
#import "TournamentSession.h"
#import "TournamentDaemon.h"
#import "NSObject+FBKVOController.h"

@interface Document ()

// Model
@property (strong) TournamentDaemon* server;
@property (strong) TournamentSession* session;
@property (strong) NSMutableDictionary* configuration;

// View Controllers
@property (strong) IBOutlet TBSeatingViewController* seatingViewController;
@property (strong) IBOutlet TBPlayersViewController* playersViewController;
@property (strong) IBOutlet TBResultsViewController* resultsViewController;

// Window Controllers
@property (strong) TBConfigurationWindowController* configurationWindowController;
@property (strong) TBPlayerWindowController* playerWindowController;

// UI Outlets
@property (weak) IBOutlet NSPopUpButton* maxPlayersButton;
@property (weak) IBOutlet NSTextField* tournamentNameField;
@property (weak) IBOutlet NSToolbarItem* tournamentNameItem;
@property (weak) IBOutlet NSView* leftPaneView;
@property (weak) IBOutlet NSView* rightPaneView;
@property (weak) IBOutlet NSView* centerPaneView;
@property (weak) IBOutlet NSWindow* mainWindow;

// Keep track of last seating plan size, to avoid setting again
@property (assign) NSInteger lastMaxPlayers;

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

        // Start serving using this device's auth key
        NSString* path = [[self server] startWithAuthCode:[TournamentSession clientIdentifier]];

        // Start the session, connecting locally
        [[self session] connectToLocalPath:path];
    }
    return self;
}

- (void)close {
    // close all windows
    [[self configurationWindowController] close];
    [[self playerWindowController] close];

    [[self KVOController] unobserveAll];
    [[self session] disconnect];
    [[self server] stop];
    [super close];
}

- (void)windowControllerDidLoadNib:(NSWindowController*)aController {
    [super windowControllerDidLoadNib:aController];

    // add subivews
    [[self seatingViewController] setSession:[self session]];
    [[self centerPaneView] addSubview:[[self seatingViewController] view]];

    [[self playersViewController] setDelegate:[self seatingViewController]];
    [[self playersViewController] setSession:[self session]];
    [[self leftPaneView] addSubview:[[self playersViewController] view]];

    [[self resultsViewController] setSession:[self session]];
    [[self rightPaneView] addSubview:[[self resultsViewController] view]];

    // whenever tournament name changes, do some stuff
    [[self KVOController] observe:[self session] keyPath:@"name" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        // re-publish on Bonjour
        [[self server] publishWithName:[object name]];

        // resize toolbar control
        [[self tournamentNameField] sizeToFit];
        NSSize size = [[self tournamentNameField] frame].size;

        // resize the toolbar item
        [[self tournamentNameItem] setMinSize:size];
        [[self tournamentNameItem] setMaxSize:size];
    }];

    // update max players selector
    [[self KVOController] observe:[self configuration] keyPath:@"players" options:0 block:^(id observer, id object, NSDictionary *change) {
        NSUInteger players = [object[@"players"] count];
        if(players > 1 && players != [[[self maxPlayersButton] lastItem] tag]) {

            // remember selection
            NSInteger selectedTag = [[self maxPlayersButton] selectedTag];

            // start with empty menu
            [[self maxPlayersButton] removeAllItems];

            NSLog(@"Populating maxPlayers button with %ld items", players);
            for(NSUInteger i=2; i<=players; i++) {
                // add item with a title corresponding to number of potential players
                [[self maxPlayersButton] addItemWithTitle:[@(i) stringValue]];
                // add tag
                [[[self maxPlayersButton] itemAtIndex:i-2] setTag:i];
            }

            // select tag
            if(selectedTag > 1) {
                [[self maxPlayersButton] selectItemWithTag:selectedTag];
            } else {
                [[self maxPlayersButton] selectItemWithTag:players];

                // also plan seating
                [self planSeatingFor:players force:NO];
            }
        }
    }];

    // if table sizes change, replan
    [[self KVOController] observe:[self configuration] keyPath:@"table_capacity" options:0 block:^(id observer, id object, NSDictionary *change) {
        NSInteger maxPlayers = [[self maxPlayersButton] selectedTag];
        [self planSeatingFor:maxPlayers force:NO];
    }];
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
    if([typeName isEqualToString:@"JSON"] || [typeName isEqualToString:@"TournamentBuddy"]) {
        // serialize json configuration to NSData
        return [NSJSONSerialization dataWithJSONObject:[self configuration] options:0 error:outError];
    } else if([typeName isEqualToString:@"CSV"]) {
        // serialize results to NSData
        NSMutableArray* results = [[NSMutableArray alloc] initWithObjects:@"Player,Finish,Win", nil];
        // 1 result per line
        [[[self session] results] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL* stop) {
            NSString* line = [NSString stringWithFormat:@"\"%@\",%@,%@", obj[@"name"], obj[@"place"], obj[@"payout"]];
            [results addObject:line];
        }];
        // combine
        NSString* csv = [results componentsJoinedByString:@"\n"];
        // use WINDOWS-1252 for now
        return [csv dataUsingEncoding:NSWindowsCP1252StringEncoding];
    }
    return nil;
}

- (BOOL)readFromData:(NSData*)data ofType:(NSString*)typeName error:(NSError**)outError {
    if([typeName isEqualToString:@"JSON"] || [typeName isEqualToString:@"TournamentBuddy"]) {
        // deserialize json configuration
        [[self configuration] setDictionary:[NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:outError]];
    }
    return *outError == nil;
}

- (NSPrintOperation*)printOperationWithSettings:(NSDictionary*)ps error:(NSError**)outError {
    NSPrintInfo* printInfo = [self printInfo];
    NSPrintOperation* printOp = [NSPrintOperation printOperationWithView:[[self seatingViewController] view] printInfo:printInfo];
    return printOp;
}

#pragma mark Operations

- (void)planSeatingFor:(NSInteger)maxPlayers force:(BOOL)forced {
    NSLog(@"Planning seating for %ld players", maxPlayers);
    if(maxPlayers > 1 && (forced || maxPlayers != [self lastMaxPlayers])) {
        [[self session] planSeatingFor:@(maxPlayers)];
        [self setLastMaxPlayers:maxPlayers];
    }
}

#pragma mark Actions

- (IBAction)exportResults:(id)sender {
    NSSavePanel* savePanel = [NSSavePanel savePanel];
    [savePanel setShowsTagField:NO];
    [savePanel setTitle:@"Export Results..."];
    [savePanel setAllowedFileTypes:@[@"CSV"]];
    [savePanel beginSheetModalForWindow:[self mainWindow] completionHandler:^(NSInteger result) {
        if(result == NSFileHandlingPanelOKButton) {
            [self saveToURL:[savePanel URL] ofType:@"CSV" forSaveOperation:NSSaveToOperation completionHandler:^(NSError* errorOrNil) {
                NSLog(@"%@", errorOrNil);
            }];
        }
    }];
}

- (IBAction)tournamentNameWasChanged:(NSTextField*)sender {
    // resign first responder
    [[sender window] selectNextKeyView:self];

    // configure session and replace current configuration
    [[self session] selectiveConfigure:[self configuration] andUpdate:[self configuration]];
    [[sender window] setDocumentEdited:YES];
}

- (IBAction)previousRoundTapped:(id)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(id)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGameAt:nil];
    }
}

- (IBAction)nextRoundTapped:(id)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(id)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        NSUInteger remaining = [[[self session] actionClockTimeRemaining] unsignedIntegerValue];
        if(remaining == 0) {
            [[self session] setActionClock:@kActionClockRequestTime];
        } else {
            [[self session] setActionClock:nil];
        }
    }
}

- (IBAction)maxPlayersTextDidChange:(id)sender {
    NSInteger maxPlayers = [sender selectedTag];
    [self planSeatingFor:maxPlayers force:NO];
}

- (IBAction)restartTapped:(id)sender {
    NSInteger maxPlayers = [[self maxPlayersButton] selectedTag];
    [self planSeatingFor:maxPlayers force:YES];
}

- (IBAction)configureButtonDidChange:(id)sender {
    if([[[self configurationWindowController] window] isVisible]) {
        [[self configurationWindowController] close];
    } else {
        // setup configuration window if needed
        if([self configurationWindowController] == nil) {
            [self setConfigurationWindowController:[[TBConfigurationWindowController alloc] initWithWindowNibName:@"TBConfigurationWindow"]];
            [[self configurationWindowController] setConfiguration:[self configuration]];
        }

        // display as a sheet
        [[self mainWindow] beginSheet:[[self configurationWindowController] window] completionHandler:^(NSModalResponse returnCode) {
            NSLog(@"Configuration sheet closed");

            switch (returnCode) {
                case NSModalResponseOK:
                    NSLog(@"Done button was pressed");
                    // configure session and replace current configuration
                    [[self session] selectiveConfigure:[[self configurationWindowController] configuration] andUpdate:[self configuration]];
                    [[self mainWindow] setDocumentEdited:YES];
                    break;
                case NSModalResponseCancel:
                    NSLog(@"Cancel button was pressed");
                    break;

                default:
                    break;
            }

        }];
    }
}

- (IBAction)displayButtonDidChange:(id)sender {
    if([[[self playerWindowController] window] isVisible]) {
        [[self playerWindowController] close];
    } else {
        // setup player window if needed
        if([self playerWindowController] == nil) {
            [self setPlayerWindowController:[[TBPlayerWindowController alloc] initWithWindowNibName:@"TBPlayerWindow"]];
            [[self playerWindowController] setSession:[self session]];
        }

        [[self playerWindowController] showWindow:self];

        // move to second screen if possible
        NSArray* screens = [NSScreen screens];
        if([screens count] > 1) {
            NSScreen* screen = [NSScreen screens][1];
            [[[self playerWindowController] window] setFrame: [screen frame] display:YES animate:NO];
            [[[self playerWindowController] window] makeKeyAndOrderFront:screen];
        }
    }
}


@end
