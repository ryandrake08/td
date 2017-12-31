//
//  TBPlayerViewController.m
//  td
//
//  Created by Ryan Drake on 6/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBPlayerViewController.h"
#import "TBActionClockView.h"
#import "TBResizeTextField.h"
#import "TBSoundPlayer.h"
#import "NSObject+FBKVOController.h"

@interface TBPlayerViewController () <TBActionClockDelegate>

// UI elements
@property (weak) IBOutlet NSImageView* backgroundImageView;
@property (weak) IBOutlet TBActionClockView* actionClockView;

// Sound player
@property (strong) IBOutlet TBSoundPlayer* soundPlayer;

// Containers
@property (strong) NSMutableArray* containerViewControllers;

@end

@implementation TBPlayerViewController

@synthesize session = _session;

- (void)viewDidLoad {
    if([NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [super viewDidLoad];
    }

    _containerViewControllers = [[NSMutableArray alloc] init];

    [[self KVOController] observe:self keyPath:@"session.state.action_clock_time_remaining" options:0 block:^(id observer, id object, NSDictionary *change) {
        NSUInteger actionClockTimeRemaining = [[[object session] state][@"action_clock_time_remaining"] unsignedIntegerValue];
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }];

    // set up sound player
    [[self soundPlayer] setSession:[self session]];
}

- (void)loadView {
    [super loadView];
    if(![NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [self viewDidLoad];
    }
}

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    id vc = [segue destinationController];
    if([vc respondsToSelector:@selector(setSession:)]) {
        [[self containerViewControllers] addObject:vc];
    }
}

- (void)setSession:(TournamentSession*)session {
    _session = session;
    for(id e in [self containerViewControllers]) {
        [e setSession:session];
    }
}

- (TournamentSession*)session {
    return _session;
}

#pragma mark TBActionClockDelegate

- (CGFloat)analogClock:(TBActionClockView*)clock graduationLengthForIndex:(NSInteger)index {
    return index % 5 == 0 ? 10.0 : 5.0;
}

@end
