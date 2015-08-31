//
//  TBAppDelegate.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBAppDelegate.h"

#import "NSObject+FBKVOController.h"

@interface TBAppDelegate ()

// keep track of when we last updated
@property (nonatomic, strong) NSDate* nextRoundNotificationUpdate;
@property (nonatomic, strong) NSDate* nextBreakNotificationUpdate;

@end

@implementation TBAppDelegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    // Override point for customization after application launch.
    if([self session] == nil) {
        [self setSession:[[TournamentSession alloc] init]];
    }

    // Handle launching from a notification
    UILocalNotification* locationNotification = [launchOptions objectForKey:UIApplicationLaunchOptionsLocalNotificationKey];
    if(locationNotification) {
        NSLog(@"Received notification while app not running");

        // Set icon badge number to zero
        application.applicationIconBadgeNumber = 0;
    }
    return YES;
}

- (void)application:(UIApplication*)application didReceiveLocalNotification:(UILocalNotification*)notification {
    UIApplicationState state = [application applicationState];

    NSLog(@"Received notification while app in state: %ld", state);

    // Set icon badge number to zero
    application.applicationIconBadgeNumber = 0;
}

- (void)applicationWillResignActive:(UIApplication*)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    NSLog(@"applicationWillResignActive");
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

    [[self KVOController] observe:[self session] keyPath:@"currentRoundText" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        // also clear schedule for notification updates
        [self setNextRoundNotificationUpdate:nil];
        [self setNextBreakNotificationUpdate:nil];
    }];

    [[self KVOController] observe:[self session] keyPath:@"timeRemaining" options:0 block:^(id observer, id object, NSDictionary *change) {
        if([self nextRoundNotificationUpdate] == nil || [[NSDate date] compare:[self nextRoundNotificationUpdate]] == NSOrderedDescending) {
            NSLog(@"Scheduling notifications for regular round, time remaining: %@", [object timeRemaining]);

            // schedule next try
            [self setNextRoundNotificationUpdate:[NSDate dateWithTimeIntervalSinceNow:60.0]];

            // is there a break next?
            NSUInteger currentLevel = [[object currentBlindLevel] unsignedIntegerValue];
            NSString* nextRoundText = ([object blindLevels][currentLevel][@"break_duration"] == nil) ? [object nextRoundText] : nil;

            // update
            NSInteger timeRemaining = [[object timeRemaining] integerValue];
            [self updateNotificationsForNextRound:nextRoundText withTimeRemaining:timeRemaining];
        }
    }];

    [[self KVOController] observe:[self session] keyPath:@"breakTimeRemaining" options:0 block:^(id observer, id object, NSDictionary *change) {
        if([self nextBreakNotificationUpdate] == nil || [[NSDate date] compare:[self nextBreakNotificationUpdate]] == NSOrderedDescending) {
            NSLog(@"Scheduling notifications for break, time remaining: %@", [object breakTimeRemaining]);

            // schedule next try
            [self setNextBreakNotificationUpdate:[NSDate dateWithTimeIntervalSinceNow:60.0]];

            // update
            NSInteger timeRemaining = [[object breakTimeRemaining] integerValue];
            [self updateNotificationsForNextRound:[object nextRoundText] withTimeRemaining:timeRemaining];
        }
    }];
}

- (void)applicationWillTerminate:(UIApplication*)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

#pragma mark Notification

- (void)updateLocalNotificationNamed:(NSString*)notificationId interval:(NSTimeInterval)interval body:(NSString*)alertBody action:(NSString*)actionTitle sound:(NSString*)soundName {
    // cancel old notification
    for (UILocalNotification* notification in [[UIApplication sharedApplication] scheduledLocalNotifications]) {
        if([[[notification userInfo] objectForKey:@"notificationId"] isEqualToString:notificationId]) {
            NSLog(@"Removing old notification:%@", [notification alertBody]);
            [[UIApplication sharedApplication] cancelLocalNotification:notification];
        }
    }

    // create new one
    UILocalNotification* localNotification = [[UILocalNotification alloc] init];
    [localNotification setFireDate:[NSDate dateWithTimeIntervalSinceNow:interval]];
    [localNotification setTimeZone:[NSTimeZone localTimeZone]];
    [localNotification setAlertBody:alertBody];
    [localNotification setAlertAction:actionTitle];
    [localNotification setSoundName:soundName];
    [localNotification setUserInfo:@{@"notificationId":notificationId}];
    NSLog(@"Creating new notification:%@ for %@", [localNotification alertBody], [localNotification fireDate]);
    [[UIApplication sharedApplication] scheduleLocalNotification:localNotification];
}

- (void)updateNotificationsForNextRound:(NSString*)nextRoundText withTimeRemaining:(NSInteger)timeRemaining {
    // different message/sounds depending on whether we are breaking next
    NSString* warningBody;
    NSString* nextBody;
    NSString* nextSound;
    if(nextRoundText == nil) {
        warningBody = @"One minute until break";
        nextBody = @"Break time";
        nextSound = @"s_break.caf";
    } else {
        warningBody = [@"One minute until next round: " stringByAppendingString:nextRoundText];
        nextBody = [@"New round: " stringByAppendingString:nextRoundText];
        nextSound = @"s_next.caf";
    }

    // if we're > kAudioWarningTime away, also schedule notification for warning
    if(timeRemaining > kAudioWarningTime) {
        [self updateLocalNotificationNamed:@"warning"
                                  interval:(timeRemaining-kAudioWarningTime)/1000.0
                                      body:warningBody
                                    action:@"Show timer"
                                     sound:@"s_warning.caf"];
    }

    // schedule notification for round change
    [self updateLocalNotificationNamed:@"next"
                              interval:timeRemaining/1000.0
                                  body:nextBody
                                action:@"Show timer"
                                 sound:nextSound];
}

@end
