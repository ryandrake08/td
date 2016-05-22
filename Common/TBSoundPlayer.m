//
//  TBSoundPlayer.m
//  td
//
//  Created by Ryan Drake on 5/22/16.
//  Copyright © 2016 HDna Studio. All rights reserved.
//

#import "TBSoundPlayer.h"
#import "TBSound.h"
#import "NSObject+FBKVOController.h"

@interface TBSoundPlayer ()

// Sounds
@property (nonatomic, strong) TBSound* startSound;
@property (nonatomic, strong) TBSound* nextSound;
@property (nonatomic, strong) TBSound* breakSound;
@property (nonatomic, strong) TBSound* warningSound;

@end

@implementation TBSoundPlayer

-(instancetype)init {
    if((self = [super init])) {
        // alloc sounds
        _startSound = [[TBSound alloc] initWithResource:@"s_start" extension:@"caf"];
        _nextSound = [[TBSound alloc] initWithResource:@"s_next" extension:@"caf"];
        _breakSound = [[TBSound alloc] initWithResource:@"s_break" extension:@"caf"];
        _warningSound = [[TBSound alloc] initWithResource:@"s_warning" extension:@"caf"];
    }
    return self;
}

-(void)setSession:(TournamentSession*)session {
    // unregister for KVO
    [[self KVOController] unobserveAll];

    // set session
    _session = session;

    // register for KVO
    [[self KVOController] observe:[[self session] state] keyPath:@"current_blind_level" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
        id old = change[@"old"];
        id new = change[@"new"];
        if(![old isEqualTo:[NSNull null]] && ![new isEqualTo:[NSNull null]]) {
            if([old isEqualTo:@0] && ![new isEqualTo:@0]) {
                // round zero to round non-zero: start
                [[self startSound] play];
            } else if(![old isEqualTo:@0] && [new isEqualTo:@0]) {
                // round non-zero to round zero: restart
                // no sound
            } else if (![old isEqualTo:[NSNull null]] && ![old isEqualTo:new]) {
                // round non-zero to round non-zero: next/prev
                [[self nextSound] play];
            }
        }
    }];

    [[self KVOController] observe:[[self session] state] keyPaths:@[@"on_break"] options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
        id old = change[@"old"];
        id new = change[@"new"];
        if(![old isEqualTo:[NSNull null]] && ![new isEqualTo:[NSNull null]]) {
            if([old isEqualTo:@NO] && [new isEqualTo:@YES]) {
                // break NO to YES
                [[self breakSound] play];
            }
        }
    }];

    [[self KVOController] observe:[[self session] state] keyPaths:@[@"time_remaining",@"break_time_remaining"] options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
        id old = change[@"old"];
        id new = change[@"new"];
        if(![old isEqualTo:[NSNull null]] && ![new isEqualTo:[NSNull null]]) {
            if([old integerValue] > kAudioWarningTime && [new integerValue] <= kAudioWarningTime && [new integerValue] != 0) {
                // time crosses kAudioWarningTime
                [[self warningSound] play];
            }
        }
    }];
}

@end
