//
//  TBRemoteClockViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRemoteClockViewController.h"
#import "TBAppDelegate.h"
#import "TBChipTableViewCell.h"
#import "TBColor+CSS.h"
#import "TBColor+ContrastTextColor.h"
#import "TBEllipseView.h"
#import "TBInvertableButton.h"
#import "TournamentSession.h"

#import "NSObject+FBKVOController.h"

@interface TBRemoteClockViewController () <UITableViewDataSource, UITableViewDelegate>

@property (nonatomic, strong) TournamentSession* session;

@property (nonatomic, weak) IBOutlet UILabel* elapsedLabel;
@property (nonatomic, weak) IBOutlet UILabel* clockLabel;
@property (nonatomic, weak) IBOutlet UILabel* currentGameLabel;
@property (nonatomic, weak) IBOutlet UILabel* currentRoundLabel;
@property (nonatomic, weak) IBOutlet UILabel* nextGameLabel;
@property (nonatomic, weak) IBOutlet UILabel* nextRoundLabel;
@property (nonatomic, weak) IBOutlet UILabel* playersLeftLabel;
@property (nonatomic, weak) IBOutlet UILabel* averageStackLabel;
@property (nonatomic, weak) IBOutlet TBInvertableButton* previousRoundButton;
@property (nonatomic, weak) IBOutlet TBInvertableButton* pauseResumeButton;
@property (nonatomic, weak) IBOutlet TBInvertableButton* nextRoundButton;
@property (nonatomic, weak) IBOutlet TBInvertableButton* callClockButton;

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

    [[self KVOController] observe:[[self session] state] keyPath:@"background_color" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        // Set the background color on the view
        NSString* backgroundColorName = object[@"background_color"];
        if(backgroundColorName != nil) {
            TBColor* color = [TBColor colorWithName:backgroundColorName];
            [[observer view] setBackgroundColor:color];

            // Set text label appearance to a complementary color
            if([[UILabel class] respondsToSelector:@selector(appearanceWhenContainedIn:)]) {
                [[UILabel appearanceWhenContainedIn:[TBRemoteClockViewController class], nil] setTextColor:[color contrastTextColor]];
            } else {
                [[UILabel appearanceWhenContainedInInstancesOfClasses:@[[TBRemoteClockViewController class]]] setTextColor:[color contrastTextColor]];
            }

            // Invert button images if dark
            BOOL dark = [color isDark];
            [[self previousRoundButton] setImageInverted:dark forState:UIControlStateNormal];
            [[self previousRoundButton] setImageInverted:dark forState:UIControlStateHighlighted];
            [[self pauseResumeButton] setImageInverted:dark forState:UIControlStateNormal];
            [[self pauseResumeButton] setImageInverted:dark forState:UIControlStateHighlighted];
            [[self nextRoundButton] setImageInverted:dark forState:UIControlStateNormal];
            [[self nextRoundButton] setImageInverted:dark forState:UIControlStateHighlighted];
            [[self callClockButton] setImageInverted:dark forState:UIControlStateNormal];
            [[self callClockButton] setImageInverted:dark forState:UIControlStateHighlighted];
        }
    }];

    // Register table view cell class
    [[self tableView] registerNib:[UINib nibWithNibName:@"TBChipTableViewCell" bundle:nil] forCellReuseIdentifier:@"ChipCell"];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark Update

- (void)updateActionClock:(NSNumber*)timeRemaining {
    NSUInteger actionClockTimeRemaining = [timeRemaining unsignedIntegerValue];
    if(actionClockTimeRemaining == 0 && [self presentedViewController] != nil) {
        [self dismissViewControllerAnimated:YES completion:nil];
    } else if(actionClockTimeRemaining > 0 && [self presentedViewController] == nil) {
        [self performSegueWithIdentifier:@"presentActionClockView" sender:self];
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
        NSUInteger remaining = [[[self session] state][@"action_clock_time_remaining"] unsignedIntegerValue];
        if(remaining == 0) {
            [[self session] setActionClock:@kActionClockRequestTime];
        } else {
            [[self session] clearActionClock];
        }
    }
}

#pragma mark UITableViewDataSource

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    if(section == 0) {
        return [super tableView:tableView numberOfRowsInSection:section];
    } else if(section == 1) {
        NSArray* availableChips = [[self session] state][@"available_chips"];
        return [availableChips count];
    } else {
        return 0;
    }
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    if([indexPath section] == 0) {
        return [super tableView:tableView cellForRowAtIndexPath:indexPath];
    } else if([indexPath section] == 1) {
        // relevent state
        NSArray* availableChips = [[self session] state][@"available_chips"];
        //NSString* backgroundColorName = [[self session] state][@"background_color"];
        //TBColor* color = [TBColor colorWithName:backgroundColorName];

        // create a cell
        TBChipTableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"ChipCell" forIndexPath:indexPath];
        [cell setChip:availableChips[indexPath.row] withInvertedImage:NO];
        return cell;
    } else {
        return nil;
    }
}

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return [super numberOfSectionsInTableView:tableView];
}

#pragma mark UITableViewDelegate

- (UITableViewCellEditingStyle)tableView:(UITableView*)tableView editingStyleForRowAtIndexPath:(NSIndexPath*)indexPath
{
    return UITableViewCellEditingStyleNone;
}

- (CGFloat)tableView:(UITableView*)tableView heightForRowAtIndexPath:(NSIndexPath*)indexPath
{
    // if dynamic section make all rows the same height as row 0
    if ([indexPath section] == 1) {
        return [super tableView:tableView heightForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:[indexPath section]]];
    } else {
        return [super tableView:tableView heightForRowAtIndexPath:indexPath];
    }
}

- (NSInteger)tableView:(UITableView*)tableView indentationLevelForRowAtIndexPath:(NSIndexPath*)indexPath
{
    // if dynamic section make all rows the same indentation level as row 0
    if ([indexPath section] == 1) {
        return [super tableView:tableView indentationLevelForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:[indexPath section]]];
    } else {
        return [super tableView:tableView indentationLevelForRowAtIndexPath:indexPath];
    }
}

@end
