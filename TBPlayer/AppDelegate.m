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

@interface AppDelegate ()

@property TBPlayerWindowController* windowController;

// the tournament session (model) object
@property TournamentSession* session;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    _session = [[TournamentSession alloc] init];
    
    _windowController = [[TBPlayerWindowController alloc] initWithWindowNibName:@"TBPlayerWindowController"];
    [_windowController setSession:_session];
    [_windowController showWindow:nil];
    [_windowController.window makeKeyAndOrderFront:nil];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

@end
