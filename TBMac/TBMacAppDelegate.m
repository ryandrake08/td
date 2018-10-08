//
//  TBMacAppDelegate.m
//  td
//
//  Created by Ryan Drake on 1/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBMacAppDelegate.h"

@interface AppDelegate ()

@property (nonatomic, strong) NSObject* idleSystemSleepDisabledActivity;
@property (nonatomic, strong) NSObject* displaySleepDisabledActivity;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void)applicationDidResignActive:(NSNotification *)aNotification {
    [self setIdleSystemSleepDisabledActivity:[[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityIdleSystemSleepDisabled reason:@"need to continue timer in the background"]];
    [[NSProcessInfo processInfo] endActivity:[self displaySleepDisabledActivity]];
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification {
    [[NSProcessInfo processInfo] endActivity:[self idleSystemSleepDisabledActivity]];
    [self setDisplaySleepDisabledActivity:[[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityIdleDisplaySleepDisabled reason:@"need to keep clock on screen even when application is idle"]];
}

@end
