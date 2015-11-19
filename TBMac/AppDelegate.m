//
//  AppDelegate.m
//  td
//
//  Created by Ryan Drake on 1/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@property (strong) NSObject* activity;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void)applicationDidResignActive:(NSNotification *)aNotification {
    if ([[NSProcessInfo processInfo] respondsToSelector:@selector(beginActivityWithOptions:reason:)]) {
        [self setActivity:[[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityIdleSystemSleepDisabled reason:@"need to continue timer in the background"]];
    }
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification {
    if ([[NSProcessInfo processInfo] respondsToSelector:@selector(endActivity:)]) {
        [[NSProcessInfo processInfo] endActivity:[self activity]];
    }
}

@end
