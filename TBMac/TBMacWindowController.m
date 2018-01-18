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
@property (weak) IBOutlet NSToolbarBadgedItem* playerMovesToolbarItem;

// Viewer window controller
@property (strong) NSWindowController* viewerWindowController;

// current player movements
@property (strong) NSMutableArray* playerMovements;

@end

@implementation TBMacWindowController

- (void)windowDidLoad {
    [super windowDidLoad];

    // create player movements
    [self setPlayerMovements:[[NSMutableArray alloc] init]];

    // setup player window
    NSStoryboard* viewerStoryboard = [NSStoryboard storyboardWithName:@"TBViewer" bundle:[NSBundle mainBundle]];
    [self setViewerWindowController:[viewerStoryboard instantiateInitialController]];

    // register for KVO
    [[self KVOController] observe:self keyPath:@"document" options:0 block:^(id observer, TBMacWindowController* object, NSDictionary* change) {
        // setup player view controller
        id viewerViewController = (TBViewerViewController*)[[object viewerWindowController] contentViewController];
        [viewerViewController setRepresentedObject:[(TBMacDocument*)[object document] session]];
    }];

    // register for notifications
    [[NSNotificationCenter defaultCenter] addObserverForName:kMovementsUpdatedNotification object:nil queue:nil usingBlock:^(NSNotification* note) {
        // add movements
        [[self playerMovements] addObjectsFromArray:[note object]];

        // update badge
        NSUInteger movements = [[self playerMovements] count];
        if(movements > 0) {
            [[self playerMovesToolbarItem] setBadgeValue:[NSString stringWithFormat:@"%u", (unsigned)movements]];
        } else {
            [[self playerMovesToolbarItem] setBadgeValue:nil];
        }
    }];
}

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    // pass any needed data to view controllers
    if([[segue identifier] isEqualToString:@"presentAuthCodeView"]) {
    } else if([[segue identifier] isEqualToString:@"presentPlanView"]) {
        // pass current max expected players to plan view
        TBPlanViewController* vc = (TBPlanViewController*)[segue destinationController];
        id maxExpected = [[(TBMacDocument*)[self document] session] state][@"max_expected_players"];
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
        [[self playerMovesToolbarItem] setBadgeValue:nil];
    }
}

#pragma mark NSWindowDelegate

- (void)windowWillClose:(NSNotification *)notification {
    // close all other windows
    [[self viewerWindowController] close];
}

#pragma mark Actions

- (IBAction)displayButtonDidChange:(id)sender {
    if([[[self viewerWindowController] window] isVisible]) {
        // close viewer window
        [[self viewerWindowController] close];
    } else {
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

@end
