//
//  TBRemoteClockViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRemoteClockViewController.h"
#import "TournamentSession.h"
#import "TBActionClockView.h"
#import "TBEllipseView.h"
#import "TBColor+CSS.h"
#import "TBAppDelegate.h"

#import "NSObject+FBKVOController.h"

@interface TBRemoteClockViewController () <UITableViewDataSource, TBActionClockDelegate>

@property (nonatomic, strong) TournamentSession* session;

@property (nonatomic, weak) IBOutlet UILabel* elapsedLabel;
@property (nonatomic, weak) IBOutlet UILabel* clockLabel;
@property (nonatomic, weak) IBOutlet UILabel* currentGameLabel;
@property (nonatomic, weak) IBOutlet UILabel* currentRoundLabel;
@property (nonatomic, weak) IBOutlet UILabel* nextGameLabel;
@property (nonatomic, weak) IBOutlet UILabel* nextRoundLabel;
@property (nonatomic, weak) IBOutlet UILabel* playersLeftLabel;
@property (nonatomic, weak) IBOutlet UILabel* averageStackLabel;
@property (nonatomic, weak) IBOutlet UIButton* previousRoundButton;
@property (nonatomic, weak) IBOutlet UIButton* pauseResumeButton;
@property (nonatomic, weak) IBOutlet UIButton* nextRoundButton;
@property (nonatomic, weak) IBOutlet UIButton* callClockButton;
@property (nonatomic, weak) IBOutlet TBActionClockView* actionClockView;
@property (nonatomic, weak) IBOutlet UITableView* tableView;
@property (nonatomic, weak) IBOutlet UIView* tableHeaderView;


@end

@implementation TBRemoteClockViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // register for KVO
    [[[self tableView] KVOController] observe:[[self session] state] keyPath:@"available_chips" options:0 action:@selector(reloadData)];

    [[self KVOController] observe:[[self session] state] keyPaths:@[@"connected", @"authorized", @"current_blind_level"] options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        BOOL authorized = [object[@"connected"] boolValue] && [object[@"authorized"] boolValue];
        BOOL playing = [object[@"current_blind_level"] unsignedIntegerValue] != 0;
        [[observer previousRoundButton] setEnabled:authorized && playing];
        [[observer pauseResumeButton] setEnabled:authorized];
        [[observer nextRoundButton] setEnabled:authorized && playing];
        [[observer callClockButton] setEnabled:authorized && playing];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"elapsed_time_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer elapsedLabel] setText:object[@"elapsed_time_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"clock_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer clockLabel] setText:object[@"clock_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"current_game_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer currentGameLabel] setText:object[@"current_game_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"current_round_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        NSString* currentRoundText = object[@"current_round_text"];
        if([[self currentRoundLabel] frame].size.width <= 400.0f) {
            // wrap at ante
            currentRoundText = [currentRoundText stringByReplacingOccurrencesOfString:@" A:" withString:@"\nA:"];
        }
        [[observer currentRoundLabel] setText:currentRoundText];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"next_game_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer nextGameLabel] setText:object[@"next_game_text"]];
    }];
    
    [[self KVOController] observe:[[self session] state] keyPath:@"next_round_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer nextRoundLabel] setText:object[@"next_round_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"players_left_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer playersLeftLabel] setText:object[@"players_left_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"average_stack_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer averageStackLabel] setText:object[@"average_stack_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"action_clock_time_remaining" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [observer updateActionClock:object[@"action_clock_time_remaining"]];
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark Update

- (void)updateActionClock:(NSNumber*)timeRemaining {
    NSUInteger actionClockTimeRemaining = [timeRemaining unsignedIntegerValue];
    if(actionClockTimeRemaining == 0) {
        [[self actionClockView] setHidden:YES];
    } else {
        [[self actionClockView] setHidden:NO];
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }
}

#pragma mark Actions

- (IBAction)previousRoundTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGame];
    }
}

- (IBAction)nextRoundTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        NSUInteger remaining = [[[self session] state][@"action_clock_remaining"] unsignedIntegerValue];
        if(remaining == 0) {
            [[self session] setActionClock:@kActionClockRequestTime];
        } else {
            [[self session] clearActionClock];
        }
    }
}

#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 2;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    if(section == 0) {
        return 1;
    } else if(section == 1) {
        return [[[self session] state][@"available_chips"] count];
    } else {
        return 0;
    }
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    if([indexPath section] == 0) {
        return [[self tableHeaderView] frame].size.height;
    } else if([indexPath section] == 1) {
        return 44.0f;
    } else {
        return 0.0f;
    }
}

#if 0
- (CGFloat)tableView:(UITableView*)tableView heightForHeaderInSection:(NSInteger)section {
    if(section == 0) {
        return [[self tableHeaderView] frame].size.height;
    } else {
        return 0.0;
    }
}
- (UIView*)tableView:(UITableView*)tableView viewForHeaderInSection:(NSInteger)section {
    if(section == 0) {
        return [self tableHeaderView];
    } else {
        return nil;
    }
}
#endif
- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell;
    if([indexPath section] == 0) {
        // create a cell and add it as a subview
        cell = [tableView dequeueReusableCellWithIdentifier:@"BlankCell"];
        [[cell contentView] addSubview:[self tableHeaderView]];

        // use manually set constraints
        [[self tableHeaderView] setTranslatesAutoresizingMaskIntoConstraints:NO];

        // manually build up some constraints for the view
        NSLayoutConstraint* equalWidth = [NSLayoutConstraint constraintWithItem:[self tableHeaderView]
                                                                      attribute:NSLayoutAttributeWidth
                                                                      relatedBy:0
                                                                         toItem:[cell contentView]
                                                                      attribute:NSLayoutAttributeWidth
                                                                     multiplier:1.0f
                                                                       constant:0.0f];
        NSLayoutConstraint* equalHeight = [NSLayoutConstraint constraintWithItem:[self tableHeaderView]
                                                                       attribute:NSLayoutAttributeHeight
                                                                       relatedBy:0
                                                                          toItem:[cell contentView]
                                                                       attribute:NSLayoutAttributeHeight
                                                                      multiplier:1.0f
                                                                        constant:0.0f];
        NSLayoutConstraint* centerX = [NSLayoutConstraint constraintWithItem:[self tableHeaderView]
                                                                   attribute:NSLayoutAttributeCenterX
                                                                   relatedBy:0
                                                                      toItem:[cell contentView]
                                                                   attribute:NSLayoutAttributeCenterX
                                                                  multiplier:1.0f
                                                                    constant:0.0f];
        NSLayoutConstraint* centerY = [NSLayoutConstraint constraintWithItem:[self tableHeaderView]
                                                                   attribute:NSLayoutAttributeCenterY
                                                                   relatedBy:0
                                                                      toItem:[cell contentView]
                                                                   attribute:NSLayoutAttributeCenterY
                                                                  multiplier:1.0f
                                                                    constant:0.0f];
        // set constraint
        [[cell contentView] addConstraints:@[equalWidth, equalHeight, centerX, centerY]];
    } else if([indexPath section] == 1) {
        // create a cell
        cell = [tableView dequeueReusableCellWithIdentifier:@"ChipCell" forIndexPath:indexPath];

        // get result for this row
        NSDictionary* chip = [[self session] state][@"available_chips"][indexPath.row];

        // setup cell
        [(TBEllipseView*)[cell viewWithTag:100] setColor:[TBColor colorWithName:chip[@"color"]]];
        [(UILabel*)[cell viewWithTag:102] setText:[chip[@"denomination"] stringValue]];
    }
    return cell;
}

#pragma mark TBActionClockViewDelegate

- (CGFloat)analogClock:(TBActionClockView*)clock graduationLengthForIndex:(NSInteger)index {
    return index % 5 == 0 ? 10.0 : 5.0;
}

@end
