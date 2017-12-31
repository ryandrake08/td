//
//  TBPlayerViewController.m
//  td
//
//  Created by Ryan Drake on 6/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBPlayerViewController.h"
#import "TBActionClockViewController.h"
#import "TBResizeTextField.h"
#import "TBSoundPlayer.h"
#import "NSObject+FBKVOController.h"

@interface TBPlayerViewController ()

// UI elements
@property (weak) IBOutlet NSImageView* backgroundImageView;

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
        [observer updateActionClock:[[object session] state][@"action_clock_time_remaining"]];
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
    id dc = [segue destinationController];
    if([dc respondsToSelector:@selector(setSession:)]) {
        [[self containerViewControllers] addObject:dc];
    }

    if([[segue identifier] isEqualToString:@"presentActionClockView"]) {
        TBActionClockViewController* vc =  [segue destinationController];
        [vc setSession:[self session]];
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

#pragma mark Update action clock

- (void)updateActionClock:(NSNumber*)timeRemaining {
    NSUInteger actionClockTimeRemaining = [timeRemaining unsignedIntegerValue];
    NSArray* presented = [self presentedViewControllers];
    if(actionClockTimeRemaining == 0 && [presented count] != 0) {
        [self dismissViewController:presented[0]];
    } else if(actionClockTimeRemaining > 0 && [presented count] == 0) {
        [self performSegueWithIdentifier:@"presentActionClockView" sender:self];
    }
}

@end
