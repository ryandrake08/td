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

@property TBPlayerWindowController* windowController;

// the tournament session (model) object
@property TournamentSession* session;

// a tournament broswer
@property TournamentBrowser* browser;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    // create a session
    _session = [[TournamentSession alloc] init];

    // create a browser
    _browser = [[TournamentBrowser alloc] initWithDelegate:self];
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

    if([[browser localServiceList] count]) {
        NSMenuItem* headerItem = [connectMenu addItemWithTitle:NSLocalizedString(@"Local Tournaments", nil) action:nil keyEquivalent:@""];
        [headerItem setEnabled:NO];
        for(NSString* name in [browser localServiceList]) {
            NSMenuItem* serviceItem = [connectMenu addItemWithTitle:[name lastPathComponent]
                                                             action:@selector(connectToLocal:)
                                                      keyEquivalent:@""];
            [serviceItem setTarget:self];
            [serviceItem setRepresentedObject:name];
        }
        [connectMenu addItem:[NSMenuItem separatorItem]];
    }

    if([[browser remoteServiceList] count]) {
        NSMenuItem* headerItem = [connectMenu addItemWithTitle:NSLocalizedString(@"Remote Tournaments", nil) action:nil keyEquivalent:@""];
        [headerItem setEnabled:NO];
        for(NSNetService* service in [browser remoteServiceList]) {
            NSMenuItem* serviceItem = [connectMenu addItemWithTitle:[service name]
                                                             action:@selector(connectToRemote:)
                                                      keyEquivalent:@""];
            [serviceItem setTarget:self];
            [serviceItem setRepresentedObject:service];
        }
        [connectMenu addItem:[NSMenuItem separatorItem]];
    }

    NSMenuItem* connectToServiceItem = [connectMenu addItemWithTitle:NSLocalizedString(@"Connect to Service...", @"")
                                                              action:@selector(connectToTournament:)
                                                       keyEquivalent:@"C"];
    [connectToServiceItem setTarget:self];
    [connectToServiceItem setKeyEquivalentModifierMask:NSCommandKeyMask|NSShiftKeyMask];

    // set the new submenu
    [[self connectMenuItem] setSubmenu:connectMenu];
}

- (IBAction)connectToLocal:(id)sender {
    [[self session] connectToLocalPath:[sender representedObject]];
}

- (IBAction)connectToRemote:(id)sender {
    [[self session] connectToService:[sender representedObject]];
}

- (IBAction)connectToTournament:(id)sender {
}

#pragma mark TournamentBroswerDelegate

- (void)tournamentBrowser:(TournamentBrowser*)tournamentBroswer didUpdateRemoteServices:(NSArray*)services {
    [self updateMenuWithBrowser:tournamentBroswer];
}

@end
