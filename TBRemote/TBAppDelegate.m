//
//  TBAppDelegate.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBAppDelegate.h"
#import "NSObject+FBKVOController.h"

@import WatchConnectivity;

@interface TBAppDelegate () <WCSessionDelegate>

@end

@implementation TBAppDelegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    // Override point for customization after application launch.
    if([self session] == nil) {
        [self setSession:[[TournamentSession alloc] init]];
    }

    // Create either tournamnet list or setup, depending on whether this is the full version or the remote
    UIStoryboard* sb = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    NSString* firstTabName = (YES) ? @"TBTournamentsViewController" : @"TBSettingsViewController";
    UIViewController* vc = [sb instantiateViewControllerWithIdentifier:firstTabName];

    // Add it as first tab
    UITabBarController* root = (UITabBarController*)[[self window] rootViewController];
    NSMutableArray* tabs = [NSMutableArray arrayWithArray:[root viewControllers]];
    [tabs insertObject:vc atIndex:0];
    [root setViewControllers:tabs];

    // Register local notifications
    if([UIApplication instancesRespondToSelector:@selector(registerUserNotificationSettings:)]) {
        UIUserNotificationSettings* settings = [UIUserNotificationSettings settingsForTypes:UIUserNotificationTypeAlert|UIUserNotificationTypeSound categories:nil];
        [application registerUserNotificationSettings:settings];
    }

    // Handle launching from a notification
    UILocalNotification* locationNotification = launchOptions[UIApplicationLaunchOptionsLocalNotificationKey];
    if(locationNotification) {
        NSLog(@"Received notification while app not running");
    }

    // WCSession
    if([WCSession isSupported]) {
        WCSession* session = [WCSession defaultSession];
        [session setDelegate:self];
        [session activateSession];
    }
    return YES;
}

- (void)application:(UIApplication*)application didReceiveLocalNotification:(UILocalNotification*)notification {
    UIApplicationState state = [application applicationState];

    NSLog(@"Received notification while app in state: %ld", (long)state);
    if(state != UIApplicationStateActive) {
        // switch to clock tab if app is inactive or running in the background
        [(UITabBarController*)[[self window] rootViewController] setSelectedIndex:1];
    }
}

- (void)applicationWillResignActive:(UIApplication*)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    NSLog(@"applicationWillResignActive");

    // schedule round notification
    [self updateNotificationsForSessionState:[[self session] state]];

    // stop observing KVO while not active
    [[self KVOController] unobserveAll];
}

- (void)applicationDidEnterBackground:(UIApplication*)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication*)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication*)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    NSLog(@"applicationDidBecomeActive");

    // KVO for notifications
    [[self KVOController] observe:[[self session] state] keyPaths:@[@"running", @"current_round_text"] options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        // schedule round notification (next runloop)
        [self performSelector:@selector(updateNotificationsForSessionState:) withObject:[[self session] state] afterDelay:0.0];
    }];

    // Notification center - Update WCSession when certain state changes happen
    [[NSNotificationCenter defaultCenter] addObserverForName:TournamentSessionUpdatedNotification object:nil queue:nil usingBlock:^(NSNotification* note) {
        NSMutableDictionary* update = [NSMutableDictionary dictionaryWithDictionary:[note object]];
        [update removeObjectsForKeys:@[@"elapsed_time", @"elapsed_time_text", @"time_remaining", @"break_time_remaining", @"action_clock_time_remaining"]];
        if([update count] > 0) {
            [[WCSession defaultSession] sendMessage:@{@"state":update} replyHandler:nil errorHandler:nil];
        }
    }];
}

- (void)applicationWillTerminate:(UIApplication*)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

#pragma mark Notification

- (void)updateNotificationsForSessionState:(NSDictionary*)state {
    // relevant session variables
    BOOL running = [state[@"running"] boolValue];
    NSInteger timeRemaining = [state[@"time_remaining"] integerValue];
    NSInteger breakTimeRemaining = [state[@"break_time_remaining"] integerValue];
    NSString* nextRoundText = state[@"next_round_text"];

    // first, cancel
    [[UIApplication sharedApplication] cancelAllLocalNotifications];

    // schedule new notification if the clock is running
    if(running && (timeRemaining > 0 || breakTimeRemaining > 0)) {
        NSTimeInterval interval;
        NSString* alertBody;
        NSString* alertTitle;
        NSString* soundName;

        // four possible notifications
        if(timeRemaining > kAudioWarningTime) {
            // more than one minute of play left in round
            interval = (timeRemaining - kAudioWarningTime) / 1000.0;
            soundName = @"s_warning.caf";
            alertTitle = NSLocalizedString(@"One minute", nil);
            if(breakTimeRemaining > 0) {
                alertBody = NSLocalizedString(@"One minute until break", nil);
            } else {
                alertBody = [NSLocalizedString(@"One minute until next round: ", nil) stringByAppendingString:nextRoundText];
            }
        } else if(timeRemaining > 0) {
            // less than one minute of play left in round
            interval = timeRemaining / 1000.0;
            if(breakTimeRemaining > 0) {
                soundName = @"s_break.caf";
                if(breakTimeRemaining < 60000) {
                    NSInteger seconds = breakTimeRemaining / 1000;
                    alertBody = [NSString stringWithFormat:NSLocalizedString(@"Players are now on a %ld second break", nil), seconds];
                } else if(breakTimeRemaining < 3600000) {
                    NSInteger minutes = breakTimeRemaining / 60000;
                    alertBody = [NSString stringWithFormat:NSLocalizedString(@"Players are now on a %ld minute break", nil), minutes];
                } else {
                    NSInteger minutes = (breakTimeRemaining / 60000) % 60;
                    NSInteger hours = (breakTimeRemaining / 3600000);
                    alertBody = [NSString stringWithFormat:NSLocalizedString(@"Players are now on a %ld hour, %ld minute break", nil), hours, minutes];
                }
                alertTitle = NSLocalizedString(@"Break time", nil);
            } else {
                soundName = @"s_next.caf";
                alertBody = [NSLocalizedString(@"New round: ", nil) stringByAppendingString:nextRoundText];
                alertTitle = [nextRoundText copy];
            }
        } else if(breakTimeRemaining > kAudioWarningTime) {
            // more than one minute left in break
            interval = (breakTimeRemaining - kAudioWarningTime) / 1000.0;
            soundName = @"s_warning.caf";
            alertBody = [NSLocalizedString(@"One minute until next round: ", nil) stringByAppendingString:nextRoundText];
            alertTitle = NSLocalizedString(@"One minute", nil);
        } else if(breakTimeRemaining > 0) {
            // less than one minute left in break
            interval = breakTimeRemaining / 1000.0;
            soundName = @"s_next.caf";
            alertBody = [NSLocalizedString(@"New round: ", nil) stringByAppendingString:nextRoundText];
            alertTitle = [nextRoundText copy];
        }

        // create and schedule local notification
        UILocalNotification* localNotification = [[UILocalNotification alloc] init];
        [localNotification setTimeZone:[NSTimeZone localTimeZone]];
        [localNotification setAlertAction:NSLocalizedString(@"Show timer", @"Launch app and show tournament timer")];
        [localNotification setFireDate:[NSDate dateWithTimeIntervalSinceNow:interval]];
        [localNotification setAlertBody:alertBody];
        [localNotification setSoundName:soundName];
        NSLog(@"Creating new notification:%@ for %@", [localNotification alertBody], [localNotification fireDate]);
        [[UIApplication sharedApplication] scheduleLocalNotification:localNotification];
    }
}

#pragma mark WCSessionDelegate

- (void)session:(WCSession*)session didReceiveMessage:(NSDictionary<NSString*,id>*)message {
    NSString* command = message[@"command"];
    if(command) {
        NSLog(@"Command received from watch: %@", command);

        if([command isEqualToString:@"previousRound"]) {
            NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
            if(currentBlindLevel != 0) {
                [[self session] setPreviousLevelWithBlock:nil];
            }
        }

        if([command isEqualToString:@"playPause"]) {
            NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
            if(currentBlindLevel != 0) {
                [[self session] togglePauseGame];
            } else {
                [[self session] startGameAt:nil];
            }
        }

        if([command isEqualToString:@"nextRound"]) {
            NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
            if(currentBlindLevel != 0) {
                [[self session] setNextLevelWithBlock:nil];
            }
        }

        if([command isEqualToString:@"callClock"]) {
            NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
            if(currentBlindLevel != 0) {
                NSUInteger remaining = [[[self session] state][@"action_clock_time_remaining"] unsignedIntegerValue];
                if(remaining == 0) {
                    [[self session] setActionClock:@kActionClockRequestTime];
                } else {
                    [[self session] setActionClock:nil];
                }
            }
        }
    }
}

@end
