//
//  TBAppDelegate.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBAppDelegate.h"
#import "NSObject+FBKVOController.h"
#import "TBRemoteWatchDelegate.h"
#import "TBUserNotificationDelegate.h"
#import "TBNotificationAttributes.h"
#import "TBSettingsViewController.h"
#import "TournamentSession.h"
#import "UIResponder+PresentingErrors.h"

@interface TBAppDelegate () <TournamentSessionDelegate>

@property (nonatomic, strong) TBRemoteWatchDelegate* watchDelegate;
@property (nonatomic, strong) TBUserNotificationDelegate* notificationDelegate;

// Background task keeps server running for some time while in the background
@property (nonatomic, assign)UIBackgroundTaskIdentifier backgroundTask;

@end

@implementation TBAppDelegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    // initialize background task
    [self setBackgroundTask:UIBackgroundTaskInvalid];

    // get a session
    [self setSession:[[TournamentSession alloc] init]];

    // set tournament session delegate
    [[self session] setDelegate:self];

#if defined(TBFULL)
    // use setup storyboard in the full version
    UIStoryboard* sb = [UIStoryboard storyboardWithName:@"Setup" bundle:nil];

    // map out controller hierarchy
    UITabBarController* rootViewController = [self rootViewController];
    UISplitViewController* splitController = (UISplitViewController*)[sb instantiateInitialViewController];
    UINavigationController* navController = [[splitController viewControllers] firstObject];
    TBSettingsViewController* settingViewController = (TBSettingsViewController*)[[navController viewControllers] firstObject];

    // set up the split view controller
    [splitController setPreferredDisplayMode:UISplitViewControllerDisplayModeAllVisible];

    // add it as first tab
    NSMutableArray* tabs = [NSMutableArray arrayWithArray:[rootViewController viewControllers]];
    tabs[0] = splitController;
    [rootViewController setViewControllers:tabs];

    // handle launching with a URL
    NSURL* url = launchOptions[UIApplicationLaunchOptionsURLKey];
    if(url != nil) {
        // url passed as an app launch option. use file at url
        [settingViewController loadDocumentFromContentsOfURL:url];
    } else {
        // no url passed. use a default file for now and create it if missing
        [settingViewController loadDocumentFromContentsOfFile:@"mobile.pokerbuddy"];
    }
#endif

    // Notification delegate (iOS10+)
    if(@available(iOS 10.0, *)) {
        [self setNotificationDelegate:[[TBUserNotificationDelegate alloc] initWithHandler:^{
            // switch to clock tab when notification happens
            [[self rootViewController] setSelectedIndex:2];
        }]];
    } else if([UIApplication instancesRespondToSelector:@selector(registerUserNotificationSettings:)]) {
        // old-style UILocalNotification
        UIUserNotificationSettings* settings = [UIUserNotificationSettings settingsForTypes:UIUserNotificationTypeAlert|UIUserNotificationTypeSound categories:nil];
        [application registerUserNotificationSettings:settings];
    }

    // Watch delegate (iOS9+)
    [self setWatchDelegate:[[TBRemoteWatchDelegate alloc] initWithSession:[self session]]];

    return YES;
}

- (void)application:(UIApplication*)application didReceiveLocalNotification:(UILocalNotification*)notification {
    UIApplicationState state = [application applicationState];

    NSLog(@"Received notification while app in state: %ld", (long)state);
    if(state != UIApplicationStateActive) {
        // switch to clock tab if app is inactive or running in the background
        [[self rootViewController] setSelectedIndex:2];
    }
}

- (void)applicationWillResignActive:(UIApplication*)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    NSLog(@"applicationWillResignActive");

    // KVO for notifications
    [[self KVOController] observe:self keyPaths:@[@"session.state.running", @"session.state.next_round_text"] options:NSKeyValueObservingOptionInitial block:^(id observer, TBAppDelegate* object, NSDictionary* change) {
        // schedule round notification
        NSLog(@"scheduling round notification because app is inactive and %@ changed", change[FBKVONotificationKeyPathKey]);

        // get notification attributes based on timer state
        TBNotificationAttributes* attributes = [[TBNotificationAttributes alloc] initWithTournamentState:[[object session] state]  warningTime:kAudioWarningTime];

        if(@available(iOS 10.0, *)) {
            [[self notificationDelegate] setNotificationAttributes:attributes];
        } else {
            // old-style UILocalNotification
            [[UIApplication sharedApplication] cancelAllLocalNotifications];

            if(([attributes title] != nil) && ([attributes body] != nil) && ([attributes soundName] != nil) && ([attributes date] != nil)) {
                // create and schedule old-style local notification
                UILocalNotification* localNotification = [[UILocalNotification alloc] init];
                if(@available(iOS 8.2, *)) {
                    [localNotification setAlertTitle:[attributes title]];
                }
                [localNotification setAlertBody:[attributes body]];
                [localNotification setSoundName:[attributes soundName]];
                [localNotification setTimeZone:[NSTimeZone localTimeZone]];
                [localNotification setFireDate:[attributes date]];
                [localNotification setAlertAction:NSLocalizedString(@"Show timer", @"Launch app and show tournament timer")];
                NSLog(@"Creating new notification:%@ for %@", [localNotification alertBody], [localNotification fireDate]);
                [[UIApplication sharedApplication] scheduleLocalNotification:localNotification];
            }
        }
    }];
}

- (void)applicationDidEnterBackground:(UIApplication*)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    NSLog(@"applicationDidEnterBackground");

#if defined(TBFULL)
    // Start a background task here, this will keep client connections alive for a short period while app is in the background
    __block UIBackgroundTaskIdentifier background_task = [application beginBackgroundTaskWithExpirationHandler: ^ {
        NSLog(@"Background task expired");
        [application endBackgroundTask:background_task];
        background_task = UIBackgroundTaskInvalid;
    }];

    // Store it so we can end if app enters foreground
    [self setBackgroundTask:background_task];
#endif
}

- (void)applicationWillEnterForeground:(UIApplication*)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
    NSLog(@"applicationWillEnterForeground");

    // End any background task here
    if([self backgroundTask] != UIBackgroundTaskInvalid) {
        [application endBackgroundTask:[self backgroundTask]];
        [self setBackgroundTask:UIBackgroundTaskInvalid];
    }
}

- (void)applicationDidBecomeActive:(UIApplication*)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    NSLog(@"applicationDidBecomeActive");

    // stop observing KVO while not active
    [[self KVOController] unobserveAll];
}

- (void)applicationWillTerminate:(UIApplication*)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

#pragma mark TournamentSessionDelegate

- (void)tournamentSession:(TournamentSession *)ts error:(NSError *)error {
    // Default error presentation
    [[self rootViewController] presentError:error];
}

#pragma mark Accessors

- (UITabBarController*)rootViewController {
    return (UITabBarController*)[[self window] rootViewController];
}

@end
