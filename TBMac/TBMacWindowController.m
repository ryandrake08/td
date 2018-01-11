//
//  TBMacWindowController.m
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBMacWindowController.h"
#import "NSObject+FBKVOController.h"
#import "NSToolbarBadgedItem.h"
#import "TBMacDocument.h"
#import "TBMovementViewController.h"
#import "TBNotifications.h"
#import "TBPlanViewController.h"
#import "TBViewerViewController.h"
#import "TournamentSession.h"

@interface TBMacWindowController () <NSWindowDelegate>

// ui
@property (weak) IBOutlet NSTextField* tournamentNameField;
@property (weak) IBOutlet NSToolbarItem* tournamentNameItem;
@property (weak) IBOutlet NSToolbarBadgedItem* rebalanceToolbarItem;

// Viewer window controller
@property (strong) NSWindowController* viewerWindowController;

// current player movements
@property (strong) NSMutableArray* playerMovements;

@end

@implementation TBMacWindowController

- (TournamentSession*)session {
    // TODO: Replace this with TBMacDocument API. We should not be mutating the session
    return [(TBMacDocument*)[self document] session];
}

- (void)windowDidLoad {
    [super windowDidLoad];

    // create player movements
    [self setPlayerMovements:[[NSMutableArray alloc] init]];

    // setup player window
    NSStoryboard* viewerStoryboard = [NSStoryboard storyboardWithName:@"TBViewer" bundle:[NSBundle mainBundle]];
    [self setViewerWindowController:[viewerStoryboard instantiateInitialController]];

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
    [[self KVOController] observe:self keyPath:@"session.state.table_capacity" options:0 block:^(id observer, TBMacDocument* object, NSDictionary *change) {
        [(TBMacDocument*)[self document] planSeating];
    }];

    // register for notifications
    [[NSNotificationCenter defaultCenter] addObserverForName:kMovementsUpdatedNotification object:nil queue:nil usingBlock:^(NSNotification* note) {
        // add movements
        [[self playerMovements] addObjectsFromArray:[note object]];

        // update badge
        NSUInteger movements = [[self playerMovements] count];
        if(movements > 0) {
            [[self rebalanceToolbarItem] setBadgeValue:[NSString stringWithFormat:@"%u", (unsigned)movements]];
        } else {
            [[self rebalanceToolbarItem] setBadgeValue:nil];
        }
    }];
}

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    // pass any needed data to view controllers
    if([[segue identifier] isEqualToString:@"presentAuthCodeView"]) {
    } else if([[segue identifier] isEqualToString:@"presentPlanView"]) {
        // pass current max expected players to plan view
        TBPlanViewController* vc = (TBPlanViewController*)[segue destinationController];
        id maxExpected = [[self session] state][@"max_expected_players"];
        [vc setEnableWarning:[maxExpected boolValue]];
        NSUInteger numberOfPlayers = [[(TBMacDocument*)[self document] configuration][@"players"] count];
        [vc setNumberOfPlayers:numberOfPlayers];
    } else if([[segue identifier] isEqualToString:@"presentConfigurationView"]) {
        // pass configuration to the configuration view
        id vc = [segue destinationController];
        [vc setRepresentedObject:[(TBMacDocument*)[self document] configuration]];
    } else if([[segue identifier] isEqualToString:@"presentMovementView"]) {
        // move movements to the view
        TBMovementViewController* vc = (TBMovementViewController*)[segue destinationController];
        [vc setPlayerMovements:[self playerMovements]];
        [[self playerMovements] removeAllObjects];

        // remove badge
        [[self rebalanceToolbarItem] setBadgeValue:nil];
    }
}

#pragma mark NSWindowDelegate
- (void)windowWillClose:(NSNotification *)notification {
    // close all other windows
    // TODO: do we really want to do this?
    [[self viewerWindowController] close];
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

    // configure session and replace current configuration
    [(TBMacDocument*)[self document] addConfiguration:@{@"name":[sender stringValue]}];
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
    [(TBMacDocument*)[self document] planSeating];
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

- (IBAction)quickStartTapped:(id)sender {
    // alert because this is a very destructive action
    NSAlert* alert = [[NSAlert alloc] init];
    [alert setAlertStyle:NSWarningAlertStyle];
    [alert addButtonWithTitle:NSLocalizedString(@"OK", nil)];
    [alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)];
    [alert setMessageText:NSLocalizedString(@"Quick Start?", nil)];

    // display a different message if the game is running
    BOOL playing = [[[self session] state][@"current_blind_level"] unsignedIntegerValue] != 0;
    if(playing) {
        [alert setInformativeText:NSLocalizedString(@"Quick Start will end the current tournament immediately, then re-seat and buy in all players.", nil)];
    } else {
        [alert setInformativeText:NSLocalizedString(@"Quick Start will clear any existing seats and buy-ins, then re-seat and buy in all players.", nil)];
    }

    // present and only perform setup if confirmed by user
    if([alert runModal] == NSAlertFirstButtonReturn) {
        [[self session] quickSetupWithBlock:nil];
    }
}

@end
