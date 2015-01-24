//
//  TBAppDelegate.h
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "TournamentKit/TournamentKit.h"

@interface TBAppDelegate : UIResponder <UIApplicationDelegate>

@property (nonatomic) UIWindow* window;

// the tournament session (model) object
@property (nonatomic) TournamentSession* session;

@end

