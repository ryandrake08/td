//
//  TBMacWindowController.m
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBMacWindowController.h"
#import "Document.h"
#import "NSObject+FBKVOController.h"
#import "TBAuthCodeWindowController.h"
#import "TBConfigurationWindowController.h"
#import "TBMovementWindowController.h"
#import "TBNotifications.h"
#import "TBPlanWindowController.h"
#import "TBViewerViewController.h"
#import "TournamentSession.h"

@interface TBMacWindowController () <NSWindowDelegate>

@property (weak) IBOutlet NSTextField* tournamentNameField;
@property (weak) IBOutlet NSToolbarItem* tournamentNameItem;

// Window Controllers
@property (strong) NSWindowController* viewerWindowController;
@property (strong) TBMovementWindowController* movementWindowController;

// Keep track of last seating plan size
@property (assign) NSUInteger lastMaxPlayers;

@end

@implementation TBMacWindowController

- (TournamentSession*)session {
    // TODO: Replace this with Document API. We should not be mutating the session
    return [(Document*)[self document] session];
}

- (void)windowDidLoad {
    [super windowDidLoad];

    // setup player window
    NSStoryboard* viewerStoryboard = [NSStoryboard storyboardWithName:@"TBViewer" bundle:[NSBundle mainBundle]];
    [self setViewerWindowController:[viewerStoryboard instantiateInitialController]];

    // setup movement window
    [self setMovementWindowController:[[TBMovementWindowController alloc] initWithWindowNibName:@"TBMovementWindow"]];

    // whenever tournament name changes, adjust toolbar
    [[self KVOController] observe:[self document] keyPath:@"session.state.name" options:NSKeyValueObservingOptionInitial block:^(id observer, TBMacWindowController* object, NSDictionary *change) {
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

    // register for notifications
    [[NSNotificationCenter defaultCenter] addObserverForName:kMovementsUpdatedNotification object:nil queue:nil usingBlock:^(NSNotification* note) {
        // add movements
        NSArray* movements = [note object];
        [[[self movementWindowController] arrayController] addObjects:movements];

        // display as non-modal
        [[self movementWindowController] showWindow:self];
    }];
}

#pragma mark NSWindowDelegate
- (void)windowWillClose:(NSNotification *)notification {
    // close all other windows
    // TODO: do we really want to do this?
    [[self viewerWindowController] close];
    [[self movementWindowController] close];
}

#pragma mark Session interaction

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
    [savePanel beginSheetModalForWindow:[self window] completionHandler:^(NSInteger result) {
        if(result == NSFileHandlingPanelOKButton) {
            [[self document] saveToURL:[savePanel URL] ofType:@"CSV" forSaveOperation:NSSaveToOperation completionHandler:^(NSError* errorOrNil) {
                NSLog(@"%@", errorOrNil);
            }];
        }
    }];
}

- (IBAction)tournamentNameWasChanged:(NSTextField*)sender {
    // resign first responder
    [[sender window] selectNextKeyView:self];

    // Get new name
    NSDictionary* config = [[NSDictionary alloc] initWithObjectsAndKeys:[sender stringValue], @"name", nil];

    // configure session and replace current configuration
    [(Document*)[self document] addConfiguration:config];
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
    [[self window] beginSheet:[wc window] completionHandler:^(NSModalResponse returnCode) {
        if(returnCode == NSModalResponseOK) {
            if([wc object] != nil) {
                // Send new authorized client to Document
                [(Document*)[self document] addAuthorizedClient:[wc object]];
            }
        }
    }];
}

- (IBAction)configureButtonDidChange:(id)sender {
    TBConfigurationWindowController* wc = [[TBConfigurationWindowController alloc] initWithWindowNibName:@"TBConfigurationWindow"];
    [wc setConfiguration:[(Document*)[self document] configuration]];
    // display as a sheet
    [[self window] beginSheet:[wc window] completionHandler:^(NSModalResponse returnCode) {
        if(returnCode == NSModalResponseOK) {
            // reconfigure document
            [(Document*)[self document] addConfiguration:[wc configuration]];
        }
    }];
}

- (IBAction)displayButtonDidChange:(id)sender {
    if([[[self viewerWindowController] window] isVisible]) {
        // close viewer window
        [[self viewerWindowController] close];
    } else {
        // setup player view controller
        id viewerViewController = (TBViewerViewController*)[[self viewerWindowController] contentViewController];
        [viewerViewController setRepresentedObject:[self session]];

        // display viewer window as non-modal
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
    [[self window] beginSheet:[wc window] completionHandler:^(NSModalResponse returnCode) {
        if(returnCode == NSModalResponseOK) {
            [self planSeatingFor:[wc numberOfPlayers]];
        }
    }];
}

- (IBAction)movementButtonDidChange:(id)sender {
    if([[[self movementWindowController] window] isVisible]) {
        // close movement window
        [[self movementWindowController] close];
    } else {
        // display as non-modal
        [[self movementWindowController] showWindow:self];
    }
}

@end
