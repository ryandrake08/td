//
//  TBSoundPlayer.m
//  td
//
//  Created by Ryan Drake on 1/15/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSoundPlayer.h"
#import "NSObject+FBKVOController.h"
#import "TBNotifications.h"
#import "TBSound.h"
#import "TournamentSession.h"

@interface TBSoundPlayer ()

// Sounds
@property (nonatomic, strong) TBSound* startSound;
@property (nonatomic, strong) TBSound* nextSound;
@property (nonatomic, strong) TBSound* breakSound;
@property (nonatomic, strong) TBSound* warningSound;
@property (nonatomic, strong) TBSound* rebalanceSound;


@end

@implementation TBSoundPlayer

- (instancetype)init {
    if(self = [super init]) {
        // alloc sounds
        _startSound = [[TBSound alloc] initWithResource:@"s_start" extension:@"caf"];
        _nextSound = [[TBSound alloc] initWithResource:@"s_next" extension:@"caf"];
        _breakSound = [[TBSound alloc] initWithResource:@"s_break" extension:@"caf"];
        _warningSound = [[TBSound alloc] initWithResource:@"s_warning" extension:@"caf"];
        _rebalanceSound = [[TBSound alloc] initWithResource:@"s_rebalance" extension:@"caf"];\
    }
    return self;
}

- (void)setSession:(TournamentSession *)session {
    _session = session;

    // register for KVO
    [[self KVOController] observe:self keyPath:@"session.state.current_blind_level" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
        id old = change[NSKeyValueChangeOldKey];
        id new = change[NSKeyValueChangeNewKey];
        if(![old isEqual:[NSNull null]] && ![new isEqual:[NSNull null]]) {
            if([old isEqual:@0] && ![new isEqual:@0]) {
                // round zero to round non-zero: start
                [[self startSound] play];
            } else if(![old isEqual:@0] && [new isEqual:@0]) {
                // round non-zero to round zero: restart
                // no sound
            } else if (![old isEqual:[NSNull null]] && ![old isEqual:new]) {
                // round non-zero to round non-zero: next/prev
                [[self nextSound] play];
            }
        }
    }];

    [[self KVOController] observe:self keyPath:@"session.state.on_break" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
        id old = change[NSKeyValueChangeOldKey];
        id new = change[NSKeyValueChangeNewKey];
        if(![old isEqual:[NSNull null]] && ![new isEqual:[NSNull null]]) {
            if([old isEqual:@NO] && [new isEqual:@YES]) {
                // break NO to YES
                [[self breakSound] play];
            }
        }
    }];

    [[self KVOController] observe:self keyPaths:@[@"session.state.time_remaining",@"session.state.break_time_remaining"] options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, id object, NSDictionary* change) {
        id old = change[NSKeyValueChangeOldKey];
        id new = change[NSKeyValueChangeNewKey];
        if(![old isEqual:[NSNull null]] && ![new isEqual:[NSNull null]]) {
            if([old integerValue] > kAudioWarningTime && [new integerValue] <= kAudioWarningTime && [new integerValue] != 0) {
                // time crosses kAudioWarningTime
                [[self warningSound] play];
            }
        }
    }];

    // register for movement notification
    [[NSNotificationCenter defaultCenter] addObserverForName:kMovementsUpdatedNotification object:nil queue:nil usingBlock:^(NSNotification* note) {
        [[self rebalanceSound] play];
    }];
}
@end
