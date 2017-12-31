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
#import "TBAuthCodeWindowController.h"
#import "TBConfigurationWindowController.h"
#import "TBPlanWindowController.h"
#import "TBMovementWindowController.h"
#import "TBSoundPlayer.h"
#import "TBNotifications.h"
#import "TBViewerViewController.h"
#import "TournamentSession.h"
#import "TournamentDaemon.h"
#import "NSObject+FBKVOController.h"

@interface Document ()

// Model
@property (strong) TournamentDaemon* server;
@property (strong) TournamentSession* session;
@property (strong) NSMutableDictionary* configuration;

// Sound player
@property (strong) TBSoundPlayer* soundPlayer;

// View Controllers
@property (strong) IBOutlet TBSeatingViewController* seatingViewController;
@property (strong) IBOutlet TBPlayersViewController* playersViewController;
@property (strong) IBOutlet TBResultsViewController* resultsViewController;

// Window Controllers
@property (strong) NSWindowController* viewerWindowController;
@property (strong) TBMovementWindowController* movementWindowController;

// UI Outlets
@property (weak) IBOutlet NSTextField* tournamentNameField;
@property (weak) IBOutlet NSToolbarItem* tournamentNameItem;
@property (weak) IBOutlet NSView* leftPaneView;
@property (weak) IBOutlet NSView* rightPaneView;
@property (weak) IBOutlet NSView* centerPaneView;
@property (weak) IBOutlet NSWindow* mainWindow;

// Keep track of last seating plan size
@property (assign) NSUInteger lastMaxPlayers;

@end

@implementation Document

- (instancetype)init {
    self = [super init];
    if (self) {
        _server = [[TournamentDaemon alloc] init];
        _session = [[TournamentSession alloc] init];
        _configuration = [[NSMutableDictionary alloc] init];
        _soundPlayer = [[TBSoundPlayer alloc] init];
        [_soundPlayer setSession:_session];

        // register for KVO
        [[self KVOController] observe:self keyPaths:@[@"session.state.connected", @"session.state.authorized"] options:0 block:^(id observer, Document* object, NSDictionary *change) {
            if([[[object session] state][@"connected"] boolValue] && [[[object session] state][@"authorized"] boolValue]) {
                NSLog(@"Connected and authorized locally");

                // on connection, send entire configuration to session, unconditionally, and then replace with whatever the session has
                NSLog(@"Synchronizing session unconditionally");
                [[self session] configure:[self configuration] withBlock:^(id json) {
                    if(![json isEqual:[self configuration]]) {
                        NSLog(@"Document differs from session");
                        [[self configuration] setDictionary:json];
                    }
                }];
            }
        }];

        // Start serving using this device's auth key
        TournamentService* service = [[self server] startWithAuthCode:[TournamentSession clientIdentifier]];

        // Start the session, connecting locally
        if(![[self session] connectToTournamentService:service]) {
            // TODO: handle error
        }
    }
    return self;
}

- (void)close {
    // close all windows
    [[self viewerWindowController] close];

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
    [[self KVOController] observe:self keyPath:@"session.state.name" options:NSKeyValueObservingOptionInitial block:^(id observer, Document* object, NSDictionary *change) {
        // re-publish on Bonjour
        [[self server] publishWithName:[[object session] state][@"name"]];

        // resize toolbar control
        [[self tournamentNameField] sizeToFit];
        NSSize size = [[self tournamentNameField] frame].size;

        // resize the toolbar item
        [[self tournamentNameItem] setMinSize:size];
        [[self tournamentNameItem] setMaxSize:size];
    }];

    // if table sizes change, replan
    [[self KVOController] observe:self keyPath:@"session.state.table_capacity" options:0 block:^(id observer, Document* object, NSDictionary *change) {
        [self planSeatingFor:[self lastMaxPlayers]];
    }];

    // setup movement window
    if([self movementWindowController] == nil) {
        [self setMovementWindowController:[[TBMovementWindowController alloc] initWithWindowNibName:@"TBMovementWindow"]];
    }
    
    // register for notifications
    [[NSNotificationCenter defaultCenter] addObserverForName:kMovementsUpdatedNotification object:nil queue:nil usingBlock:^(NSNotification* note) {
        // add movements
        NSArray* movements = [note object];
        [[[self movementWindowController] arrayController] addObjects:movements];

        // display as non-modal
        [[self movementWindowController] showWindow:self];
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
        [[[self session] state][@"results"] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL* stop) {
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

- (void)planSeatingFor:(NSUInteger)maxPlayers {
    NSLog(@"Planning seating for %lu players", (unsigned long)maxPlayers);
    if(maxPlayers > 1) {
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
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(id)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGame];
    }
}

- (IBAction)nextRoundTapped:(id)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(id)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        NSUInteger remaining = [[[self session] state][@"action_clock_time_remaining"] unsignedIntegerValue];
        if(remaining == 0) {
            [[self session] setActionClock:@kActionClockRequestTime];
        } else {
            [[self session] clearActionClock];
        }
    }
}

- (IBAction)restartTapped:(id)sender {
    [self planSeatingFor:[self lastMaxPlayers]];
}

- (IBAction)authorizeButtonDidChange:(id)sender {
    TBAuthCodeWindowController* wc = [[TBAuthCodeWindowController alloc] initWithWindowNibName:@"TBAuthCodeWindow"];
    // display as a sheet
    [[self mainWindow] beginSheet:[wc window] completionHandler:^(NSModalResponse returnCode) {
        if(returnCode == NSModalResponseOK) {
            if([wc object] != nil) {
                // new authorized client
                [[self configuration][@"authorized_clients"] addObject:[wc object]];

                // configure session and replace current configuration
                [[self session] selectiveConfigure:[self configuration] andUpdate:[self configuration]];

                // mark document as edited
                [[self mainWindow] setDocumentEdited:YES];
            }
        }
    }];
}

- (IBAction)configureButtonDidChange:(id)sender {
    TBConfigurationWindowController* wc = [[TBConfigurationWindowController alloc] initWithWindowNibName:@"TBConfigurationWindow"];
    [wc setConfiguration:[self configuration]];
    // display as a sheet
    [[self mainWindow] beginSheet:[wc window] completionHandler:^(NSModalResponse returnCode) {
        if(returnCode == NSModalResponseOK) {
            // configure session and replace current configuration
            [[self session] selectiveConfigure:[wc configuration] andUpdate:[self configuration]];

            // mark document as edited
            [[self mainWindow] setDocumentEdited:YES];
        }
    }];
}

- (IBAction)displayButtonDidChange:(id)sender {
    if([[[self viewerWindowController] window] isVisible]) {
        [[self viewerWindowController] close];
    } else {
        // setup player window if needed
        if([self viewerWindowController] == nil) {
            NSStoryboard* viewerStoryboard = [NSStoryboard storyboardWithName:@"TBViewer" bundle:[NSBundle mainBundle]];
            [self setViewerWindowController:[viewerStoryboard instantiateInitialController]];

            // player view controller
            id playerViewController = (TBViewerViewController*)[[self viewerWindowController] contentViewController];
            [playerViewController setSession:[self session]];
        }

        // display as non-modal
        [[self viewerWindowController] showWindow:self];

        // move to second screen if possible
        NSArray* screens = [NSScreen screens];
        if([screens count] > 1) {
            NSScreen* screen = [NSScreen screens][1];
            [[[self viewerWindowController] window] setFrame: [screen frame] display:YES animate:NO];
            [[[self viewerWindowController] window] makeKeyAndOrderFront:screen];
        }
    }
}

- (IBAction)planButtonDidChange:(id)sender {
    TBPlanWindowController* wc = [[TBPlanWindowController alloc] initWithWindowNibName:@"TBPlanWindow"];
    [wc setEnableWarning:[self lastMaxPlayers] > 0];
    if([self lastMaxPlayers] > 0) {
        [wc setNumberOfPlayers:[self lastMaxPlayers]];
    } else {
        [wc setNumberOfPlayers:[[[self session] state][@"players"] count]];
    }
    // display as a sheet
    [[self mainWindow] beginSheet:[wc window] completionHandler:^(NSModalResponse returnCode) {
        if(returnCode == NSModalResponseOK) {
            [self planSeatingFor:[wc numberOfPlayers]];
        }
    }];
}

- (IBAction)movementButtonDidChange:(id)sender {
    if([[[self movementWindowController] window] isVisible]) {
        [[self movementWindowController] close];
    } else {
        // display as non-modal
        [[self movementWindowController] showWindow:self];
    }
}

@end
