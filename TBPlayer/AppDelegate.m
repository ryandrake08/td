//
//  AppDelegate.m
//  TBPlayer
//
//  Created by Ryan Drake on 6/25/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "AppDelegate.h"
#import "TournamentBrowser.h"
#import "TournamentSession.h"
#import "TBPlayerWindowController.h"
#import "TBConnectToWindowController.h"
#import "TBSoundPlayer.h"

@interface AppDelegate () <TournamentBrowserDelegate>

@property (weak) IBOutlet NSMenuItem* connectMenuItem;

// the main window controller
@property (strong) TBPlayerWindowController* windowController;

// the tournament session (model) object
@property (strong) IBOutlet TournamentSession* session;

// a tournament broswer
@property (strong) IBOutlet TournamentBrowser* browser;

// Sound player
@property (strong) TBSoundPlayer* soundPlayer;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    [self updateMenuWithBrowser:[self browser]];

    // start searching for tournaments
    [[self browser] search];

    // set up the windowController
    _windowController = [[TBPlayerWindowController alloc] initWithWindowNibName:@"TBPlayerWindow"];
    [_windowController setSession:_session];
    [_windowController showWindow:nil];
    [_windowController.window makeKeyAndOrderFront:nil];

    // set up sound player
    _soundPlayer = [[TBSoundPlayer alloc] init];
    [_soundPlayer setSession:_session];
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
    NSMenuItem* connectToServiceItem = [connectMenu addItemWithTitle:NSLocalizedString(@"Connect to Tournament...", @"") action:@selector(connectToTournament:) keyEquivalent:@"C"];
    [connectToServiceItem setTarget:self];
    [connectToServiceItem setKeyEquivalentModifierMask:NSCommandKeyMask|NSShiftKeyMask];

    // always include disconneciton
    NSMenuItem* disconnectItem = [connectMenu addItemWithTitle:NSLocalizedString(@"Disconnect", @"") action:@selector(disconnect:) keyEquivalent:@"D"];
    [disconnectItem setTarget:self];
    [disconnectItem setKeyEquivalentModifierMask:NSCommandKeyMask|NSShiftKeyMask];

    // set the new submenu
    [[self connectMenuItem] setSubmenu:connectMenu];
}

- (void)connectToTournamentMenuItem:(id)sender {
    if(![[self session] connectToTournamentService:[(NSMenuItem*)sender representedObject]]) {
        // TODO: handle error
    }
}

- (IBAction)connectToTournament:(id)sender {
    // create window controller
    TBConnectToWindowController* connectWindowController = [[TBConnectToWindowController alloc] initWithWindowNibName:@"TBConnectToWindow"];
    [connectWindowController setPort:kTournamentServiceDefaultPort];

    // display as a sheet
    [[[self windowController] window] beginSheet:[connectWindowController window] completionHandler:^(NSModalResponse returnCode) {
        if(returnCode == NSModalResponseOK) {
            NSString* address = [connectWindowController address];
            NSInteger port = [connectWindowController port];
            if(![[self session] connectToAddress:address port:port]) {
                // TODO: handle error
            }
        }
    }];
}

- (IBAction)disconnect:(id)sender {
    [[self session] disconnect];
}

#pragma mark TournamentBroswerDelegate

- (void)tournamentBrowser:(TournamentBrowser*)tournamentBroswer didUpdateServices:(NSArray*)services {
    [self updateMenuWithBrowser:tournamentBroswer];
}

@end
