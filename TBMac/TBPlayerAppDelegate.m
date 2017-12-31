//
//  TBPlayerAppDelegate.m
//  TBPlayer
//
//  Created by Ryan Drake on 6/25/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBPlayerAppDelegate.h"
#import "TournamentBrowser.h"
#import "TournamentSession.h"
#import "TBPlayerViewController.h"
#import "TBConnectToViewController.h"

@interface TBPlayerAppDelegate () <TournamentBrowserDelegate>

@property (weak) IBOutlet NSMenu* connectMenu;

// the tournament session (model) object
@property (strong) IBOutlet TournamentSession* session;

// a tournament broswer
@property (strong) IBOutlet TournamentBrowser* browser;

@end

@implementation TBPlayerAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    // player view controller
    id playerViewController = (TBPlayerViewController*)[[[[NSApplication sharedApplication] mainWindow] windowController] contentViewController];
    [playerViewController setSession:[self session]];

    // update the menu
    [self updateMenuWithBrowser:[self browser]];

    // start searching for tournaments
    [[self browser] search];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
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

- (void)connectToTournamentMenuItem:(id)sender {
    if(![[self session] connectToTournamentService:[(NSMenuItem*)sender representedObject]]) {
        // TODO: handle error
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

#pragma mark TournamentBroswerDelegate

- (void)tournamentBrowser:(TournamentBrowser*)tournamentBroswer didUpdateServices:(NSArray*)services {
    [self updateMenuWithBrowser:tournamentBroswer];
}

@end
