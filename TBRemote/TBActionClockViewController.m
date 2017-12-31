//
//  TBActionClockViewController.m
//  td
//
//  Created by Ryan Drake on 12/14/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBActionClockViewController.h"
#import "TBActionClockView.h"
#import "TBAppDelegate.h"

#import "NSObject+FBKVOController.h"

@interface TBActionClockViewController () <TBActionClockDelegate>

@property (nonatomic, strong) TournamentSession* session;
@property (nonatomic, weak) IBOutlet TBActionClockView* actionClockView;

@end

@implementation TBActionClockViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // register for KVO
    [[self KVOController] observe:self keyPath:@"session.state.action_clock_time_remaining" options:NSKeyValueObservingOptionInitial block:^(id observer, TBActionClockViewController* object, NSDictionary *change) {
        [observer updateActionClock:[[object session] state][@"action_clock_time_remaining"]];
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark Update

- (void)updateActionClock:(NSNumber*)timeRemaining {
    NSUInteger actionClockTimeRemaining = [timeRemaining unsignedIntegerValue];
    if(actionClockTimeRemaining > 0) {
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }
}

- (IBAction)clearActionClock:(id)sender {
    NSLog(@"clearing Action clock");
    [[self session] clearActionClock];
    NSLog(@"cleared Action clock");
}

#pragma mark TBActionClockDelegate

- (CGFloat)analogClock:(TBActionClockView*)clock graduationLengthForIndex:(NSInteger)index {
    return index % 5 == 0 ? 10.0 : 5.0;
}

@end
