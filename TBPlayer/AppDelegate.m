//
//  AppDelegate.m
//  TBPlayer
//
//  Created by Ryan Drake on 6/25/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "AppDelegate.h"
#import "TBPlayerWindowController.h"

@interface AppDelegate ()
@property (strong) TBPlayerWindowController* windowController;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    _windowController = [[TBPlayerWindowController alloc] initWithWindowNibName:@"TBPlayerWindowController"];
    [_windowController showWindow:nil];
    [_windowController.window makeKeyAndOrderFront:nil];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

@end
