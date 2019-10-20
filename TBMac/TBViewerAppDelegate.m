//
//  TBViewerAppDelegate.m
//  td
//
//  Created by Ryan Drake on 6/25/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBViewerAppDelegate.h"
#import "NSObject+FBKVOController.h"
#import "TournamentBrowser.h"
#import "TournamentSession.h"
#import "TournamentService.h"
#import "TBViewerViewController.h"
#import "TBSeatingChartViewController.h"
#import "TBConnectToViewController.h"
#import "TBUserNotificationDelegate.h"
#import "TBNotificationAttributes.h"

@interface TBViewerAppDelegate () <TournamentBrowserDelegate, TournamentSessionDelegate>

@property (nonatomic, weak) IBOutlet NSMenu* connectMenu;

// the tournament session (model) object
@property (nonatomic, strong) IBOutlet TournamentSession* session;

// a tournament broswer
@property (nonatomic, strong) IBOutlet TournamentBrowser* browser;

// activity to keep display from sleeping
@property (nonatomic, strong) NSObject* displaySleepDisabledActivity;

// notification scheduler
@property (nonatomic, strong) TBUserNotificationDelegate* notificationDelegate;

// Viewer window controller
@property (nonatomic, strong) IBOutlet NSWindowController* viewerWindowController;

// Seating chart window controller
@property (nonatomic, strong) IBOutlet NSWindowController* seatingChartWindowController;

@end

@implementation TBViewerAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    // set tournament session delegate
    [[self session] setDelegate:self];

    // setup display and seating chart windows
    NSStoryboard* viewerStoryboard = [NSStoryboard storyboardWithName:@"TBViewer" bundle:[NSBundle mainBundle]];
    [self setViewerWindowController:[[[NSApplication sharedApplication] mainWindow] windowController]];
//    [self setViewerWindowController:[viewerStoryboard instantiateControllerWithIdentifier:@"PlayerWindowController"]];
    [self setSeatingChartWindowController:[viewerStoryboard instantiateControllerWithIdentifier:@"SeatingChartWindowController"]];

    // setup other windows' view controllers
    TBViewerViewController* viewerViewController = (TBViewerViewController*)[[self viewerWindowController] contentViewController];
    [viewerViewController setSession:[self session]];

    TBSeatingChartViewController* seatingChartViewController = (TBSeatingChartViewController*)[[self seatingChartWindowController] contentViewController];
    [seatingChartViewController setSession:[self session]];

    // update the menu
    [self updateMenuWithBrowser:[self browser]];

    // start searching for tournaments
    [[self browser] search];

    // set up notification delegate (macOS 10.14+)
    if(@available(macOS 10.14, *)) {
        [self setNotificationDelegate:[[TBUserNotificationDelegate alloc] initWithHandler:nil]];
    }
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void)applicationDidResignActive:(NSNotification *)aNotification {
    [[NSProcessInfo processInfo] endActivity:[self displaySleepDisabledActivity]];

    // KVO for notifications
    [[self KVOController] observe:self keyPaths:@[@"session.state.running", @"session.state.next_round_text"] options:NSKeyValueObservingOptionInitial block:^(id observer, TBViewerAppDelegate* object, NSDictionary* change) {
        NSLog(@"scheduling round notification because app is inactive and %@ changed", change[FBKVONotificationKeyPathKey]);

        // get notification attributes based on timer state
        TBNotificationAttributes* attributes = [[TBNotificationAttributes alloc] initWithTournamentState:[[object session] state]  warningTime:kAudioWarningTime];

        if(@available(macOS 10.14, *)) {
            [[self notificationDelegate] setNotificationAttributes:attributes];
        }
    }];

    // stop observing KVO while not active
    [[self KVOController] unobserveAll];
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification {
    [self setDisplaySleepDisabledActivity:[[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityIdleDisplaySleepDisabled reason:@"need to keep clock on screen even when application is idle"]];

    // stop observing KVO while not active
    [[self KVOController] unobserveAll];
}

- (void)updateMenuWithBrowser:(TournamentBrowser*)browser {
    // remove old local and remote services, anything not tagged
    NSInteger idx = [[self connectMenu] indexOfItemWithTag:0];
    while(idx != -1) {
        [[self connectMenu] removeItemAtIndex:idx];
        idx = [[self connectMenu] indexOfItemWithTag:0];
    }

    // split services into local and remote
    NSArray* localServices = [browser localServiceList];
    if([localServices count]) {
        [[self connectMenu] addItem:[NSMenuItem separatorItem]];
        [[self connectMenu] addItemWithTitle:NSLocalizedString(@"On this Mac", nil) action:nil keyEquivalent:@""];
        for(TournamentService* service in localServices) {
            NSMenuItem* item = [[self connectMenu] addItemWithTitle:[service name] action:@selector(connectToTournamentMenuItem:) keyEquivalent:@""];
            [item setTarget:self];
            [item setRepresentedObject:service];
        }
    }

    NSArray* remoteServices = [browser remoteServiceList];
    if([remoteServices count]) {
        [[self connectMenu] addItem:[NSMenuItem separatorItem]];
        [[self connectMenu] addItemWithTitle:NSLocalizedString(@"On the Network", nil) action:nil keyEquivalent:@""];
        for(TournamentService* service in remoteServices) {
            NSMenuItem* item = [[self connectMenu] addItemWithTitle:[service name] action:@selector(connectToTournamentMenuItem:) keyEquivalent:@""];
            [item setTarget:self];
            [item setRepresentedObject:service];
        }
    }
}

- (IBAction)connectToTournamentMenuItem:(id)sender {
    NSError* error;
    if(![[self session] connectToTournamentService:[(NSMenuItem*)sender representedObject] error:&error]) {
        [[NSApplication sharedApplication] presentError:error];
    }
}

- (IBAction)disconnect:(id)sender {
    [[self session] disconnect];
}

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    if([[segue identifier] isEqualToString:@"presentConnectToView"]) {
        TBConnectToViewController* vc = [segue destinationController];
        [vc setSession:[self session]];
        [vc setPort:kTournamentServiceDefaultPort];
    }
}

- (IBAction)displayTapped:(id)sender {
    if([[[self viewerWindowController] window] isVisible]) {
        // close viewer window
        [[self viewerWindowController] close];
    } else {
        // display viewer window as non-modal
        [[self viewerWindowController] showWindow:self];
    }
}

- (IBAction)seatingChartTapped:(id)sender {
    if([[[self seatingChartWindowController] window] isVisible]) {
        // close viewer window
        [[self seatingChartWindowController] close];
    } else {
        // display viewer window as non-modal
        [[self seatingChartWindowController] showWindow:self];
    }
}

#pragma mark TournamentSessionDelegate

- (void)tournamentSession:(TournamentSession *)ts error:(NSError *)error {
    // Default error presentation
    [[NSApplication sharedApplication] presentError:error];
}

#pragma mark TournamentBroswerDelegate

- (void)tournamentBrowser:(TournamentBrowser*)tournamentBroswer didUpdateServices:(NSArray*)services {
    [self updateMenuWithBrowser:tournamentBroswer];
}

@end
