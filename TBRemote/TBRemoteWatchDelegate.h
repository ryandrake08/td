//
//  TBRemoteWatchDelegate.h
//  td
//
//  Created by Ryan Drake on 9/8/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TournamentSession.h"

@interface TBRemoteWatchDelegate : NSObject

// Initialize this object to communicate with a supported watch
- (instancetype)initWithSession:(TournamentSession*)session;

@end
