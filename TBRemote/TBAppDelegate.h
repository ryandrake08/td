//
//  TBAppDelegate.h
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "TournamentSession.h"

@interface TBAppDelegate : UIResponder <UIApplicationDelegate>

@property (nonatomic, strong) UIWindow* window;

// the tournament session (model) object
@property (strong) TournamentSession* session;

@end

