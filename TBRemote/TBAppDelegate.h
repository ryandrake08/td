//
//  TBAppDelegate.h
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <UIKit/UIKit.h>
@class TournamentSession;

@interface TBAppDelegate : UIResponder <UIApplicationDelegate>

// the app's main window
@property (nonatomic, strong) UIWindow* window;

// the app's root view controller is a tabBarController
@property (nonatomic, strong, readonly) UITabBarController* rootViewController;

// the tournament session (model) object
@property (nonatomic, strong) TournamentSession* session;

@end

