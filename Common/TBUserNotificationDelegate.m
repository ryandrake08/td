//
//  TBUserNotificationDelegate.m
//  td
//
//  Created by Ryan Drake on 10/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBUserNotificationDelegate.h"
#import <UserNotifications/UserNotifications.h>

@interface TBUserNotificationDelegate() <UNUserNotificationCenterDelegate>

// notification handler
@property (nonatomic, copy) void (^notificationHandler)(void);

@end

@implementation TBUserNotificationDelegate

- (instancetype)initWithHandler:(void(^)(void))notificationHandler API_AVAILABLE(ios(10.0), macos(10.14)) {
    if((self = [super init])) {
        _notificationHandler = notificationHandler;

        // Register local notifications
        UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
        [center requestAuthorizationWithOptions:UNAuthorizationOptionAlert|UNAuthorizationOptionSound
                              completionHandler:^(BOOL granted, NSError* error) {
                                  if(!granted) {
                                      // TODO: Handle error
                                      NSLog(@"User did not grant app access to display notifications");
                                  }
                              }];

        // Set delegate
        [center setDelegate:self];
    }
    return self;
}

- (void)userNotificationCenter:(UNUserNotificationCenter *)center didReceiveNotificationResponse:(UNNotificationResponse*)response withCompletionHandler:(nonnull void (^)(void))completionHandler  API_AVAILABLE(ios(10.0), macos(10.14)) {
    if([[response actionIdentifier] isEqualToString:UNNotificationDefaultActionIdentifier]) {
        // call notification handler
        [self notificationHandler]();
    }

    // call completionHandler
    completionHandler();
}

// update the notification when state changes
- (void)setNotificationAttributes:(TBNotificationAttributes*)attributes API_AVAILABLE(ios(10.0), macos(10.14)) {
    // cancel existing notifications
    UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
    [center removeAllPendingNotificationRequests];

    // if we have valid attributes, schedule a new one
    if(([attributes title] != nil) && ([attributes body] != nil) && ([attributes soundName] != nil) && ([attributes date] != nil)) {
        UNMutableNotificationContent* content = [[UNMutableNotificationContent alloc] init];
        [content setTitle:[attributes title]];
        [content setBody:[attributes body]];
        [content setSound:[UNNotificationSound soundNamed:[attributes soundName]]];

        // create notification trigger
        NSDateComponents* triggerDate = [[NSCalendar currentCalendar] components:NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay | NSCalendarUnitHour | NSCalendarUnitMinute | NSCalendarUnitSecond fromDate:[attributes date]];
        UNCalendarNotificationTrigger* trigger = [UNCalendarNotificationTrigger triggerWithDateMatchingComponents:triggerDate repeats:NO];

        // schedule notification
        UNNotificationRequest* notificationRequest = [UNNotificationRequest requestWithIdentifier:@"show_timer" content:content trigger:trigger];
        [center addNotificationRequest:notificationRequest withCompletionHandler:^(NSError* error) {
            if(error != nil) {
                // TODO: Handle error
                NSLog(@"Unable to add Notification Request");
            }
        }];
    }
}

@end
