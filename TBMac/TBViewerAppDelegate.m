//
//  TBViewerAppDelegate.m
//  td
//
//  Created by Ryan Drake on 6/25/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBViewerAppDelegate.h"
#import "TournamentBrowser.h"
#import "TournamentSession.h"
#import "TournamentService.h"
#import "TBViewerViewController.h"
#import "TBConnectToViewController.h"

@interface TBViewerAppDelegate () <TournamentBrowserDelegate, TournamentSessionDelegate>

@property (nonatomic, weak) IBOutlet NSMenu* connectMenu;

// the tournament session (model) object
@property (nonatomic, strong) IBOutlet TournamentSession* session;

// a tournament broswer
@property (nonatomic, strong) IBOutlet TournamentBrowser* browser;

// activity to keep display from sleeping
@property (nonatomic, strong) NSObject* displaySleepDisabledActivity;

@end

@implementation TBViewerAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    // set tournament session delegate
    [[self session] setDelegate:self];

    // player view controller
    id playerViewController = (TBViewerViewController*)[[[[NSApplication sharedApplication] mainWindow] windowController] contentViewController];
    [playerViewController setSession:[self session]];

    // update the menu
    [self updateMenuWithBrowser:[self browser]];

    // start searching for tournaments
    [[self browser] search];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void)applicationDidResignActive:(NSNotification *)aNotification {
    if ([[NSProcessInfo processInfo] respondsToSelector:@selector(endActivity:)] && [self displaySleepDisabledActivity ]) {
        [[NSProcessInfo processInfo] endActivity:[self displaySleepDisabledActivity]];
    }
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification {
    if ([[NSProcessInfo processInfo] respondsToSelector:@selector(beginActivityWithOptions:reason:)]) {
        [self setDisplaySleepDisabledActivity:[[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityIdleDisplaySleepDisabled reason:@"need to keep clock on screen even when application is idle"]];
    }
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
