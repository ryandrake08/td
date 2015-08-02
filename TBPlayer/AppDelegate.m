//
//  AppDelegate.m
//  TBPlayer
//
//  Created by Ryan Drake on 6/25/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "AppDelegate.h"
#import "TournamentKit/TournamentKit.h"
#import "TBPlayerWindowController.h"

@interface AppDelegate () <TournamentBrowserDelegate>

@property (weak) IBOutlet NSMenuItem* connectMenuItem;

// the main window controller
@property (strong) TBPlayerWindowController* windowController;

// the tournament session (model) object
@property (strong) IBOutlet TournamentSession* session;

// a tournament broswer
@property (strong) IBOutlet TournamentBrowser* browser;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    [self updateMenuWithBrowser:[self browser]];

    // set up the windowController
    _windowController = [[TBPlayerWindowController alloc] initWithWindowNibName:@"TBPlayerWindowController"];
    [_windowController setSession:_session];
    [_windowController showWindow:nil];
    [_windowController.window makeKeyAndOrderFront:nil];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void)updateMenuWithBrowser:(TournamentBrowser*)browser {
    NSMenu* connectMenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Connect", nil)];

    // split services into local and remote
    NSArray* localServices = [browser localServiceList];
    if([localServices count]) {
        [connectMenu addItemWithTitle:NSLocalizedString(@"On this Mac", nil) action:nil keyEquivalent:@""];
        for(TournamentService* service in localServices) {
            NSMenuItem* item = [connectMenu addItemWithTitle:[service name] action:@selector(connectToTournamentMenuItem:) keyEquivalent:@""];
            [item setTarget:self];
            [item setRepresentedObject:service];
        }
        [connectMenu addItem:[NSMenuItem separatorItem]];
    }

    NSArray* remoteServices = [browser remoteServiceList];
    if([remoteServices count]) {
        [connectMenu addItemWithTitle:NSLocalizedString(@"On the Network", nil) action:nil keyEquivalent:@""];
        for(TournamentService* service in remoteServices) {
            NSMenuItem* item = [connectMenu addItemWithTitle:[service name] action:@selector(connectToTournamentMenuItem:) keyEquivalent:@""];
            [item setTarget:self];
            [item setRepresentedObject:service];
        }
        [connectMenu addItem:[NSMenuItem separatorItem]];
    }

    // always include manual connection
    NSMenuItem* connectToServiceItem = [connectMenu addItemWithTitle:NSLocalizedString(@"Connect to Service...", @"") action:@selector(connectToTournament:) keyEquivalent:@"C"];
    [connectToServiceItem setTarget:self];
    [connectToServiceItem setKeyEquivalentModifierMask:NSCommandKeyMask|NSShiftKeyMask];

    // set the new submenu
    [[self connectMenuItem] setSubmenu:connectMenu];
}

- (void)connectToTournamentMenuItem:(id)sender {
    [[self session] connect:[sender representedObject]];
}

- (IBAction)connectToTournament:(id)sender {
}

#pragma mark TournamentBroswerDelegate

- (void)tournamentBrowser:(TournamentBrowser*)tournamentBroswer didUpdateServices:(NSArray*)services {
    [self updateMenuWithBrowser:tournamentBroswer];
}

@end
