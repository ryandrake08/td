//
//  TBUserNotificationDelegate.h
//  td
//
//  Created by Ryan Drake on 10/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TBNotificationAttributes.h"

@interface TBUserNotificationDelegate : NSObject

// initialize this object to handle user notifications
- (instancetype)initWithHandler:(void(^)(void))notificationHandler API_AVAILABLE(ios(10.0), macos(10.14));

// update the notification when state changes
- (void)setNotificationAttributes:(TBNotificationAttributes*)attributes API_AVAILABLE(ios(10.0), macos(10.14));

@end
