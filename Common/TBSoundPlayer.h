//
//  TBSoundPlayer.h
//  td
//
//  Created by Ryan Drake on 5/22/16.
//  Copyright © 2016 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TournamentSession.h"

@interface TBSoundPlayer : NSObject

// the tournament session (model) object
@property (nonatomic, strong) TournamentSession* session;

@end
