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
#import "TBInvertableImageView.h"
#import "TBInvertableButton_iOS.h"
#import "TBSoundPlayer.h"
#import "TournamentSession.h"

#import "NSObject+FBKVOController.h"

@interface TBRemoteClockViewController () <UITableViewDataSource, UITableViewDelegate>

@property (nonatomic, strong) TournamentSession* session;

// sound player
@property (nonatomic, strong) TBSoundPlayer* soundPlayer;

// ui
@property (nonatomic, weak) IBOutlet UILabel* elapsedLabel;
@property (nonatomic, weak) IBOutlet UILabel* clockLabel;
@property (nonatomic, weak) IBOutlet UILabel* currentBlindsLabel;
@property (nonatomic, weak) IBOutlet UILabel* currentAnteLabel;
@property (nonatomic, weak) IBOutlet UILabel* nextRoundLabel;
@property (nonatomic, weak) IBOutlet UILabel* playersLeftLabel;
@property (nonatomic, weak) IBOutlet UILabel* averageStackLabel;
@property (nonatomic, weak) IBOutlet TBInvertableButton* previousRoundButton;
@property (nonatomic, weak) IBOutlet TBInvertableButton* pauseResumeButton;
@property (nonatomic, weak) IBOutlet TBInvertableButton* nextRoundButton;
@property (nonatomic, weak) IBOutlet TBInvertableButton* callClockButton;

@property (nonatomic, assign) BOOL backgroundIsDark;

@end

@implementation TBRemoteClockViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // set background dark mode
    _backgroundIsDark = NO;

    // set sound player
    _soundPlayer = [[TBSoundPlayer alloc] init];
    [[self soundPlayer] setSession:[self session]];

    // register for KVO
    [[[self tableView] KVOController] observe:self keyPath:@"session.state.available_chips" options:0 action:@selector(reloadData)];

    [[self KVOController] observe:self keyPaths:@[@"session.connected", @"session.authorized", @"session.state.current_blind_level"] options:NSKeyValueObservingOptionInitial block:^(id observer, TBRemoteClockViewController* object, NSDictionary *change) {
        BOOL authorized = [[object session] connected] && [[object session] authorized];
        BOOL playing = [[[object session] state][@"current_blind_level"] unsignedIntegerValue] != 0;
        [[observer previousRoundButton] setEnabled:authorized && playing];
        [[observer pauseResumeButton] setEnabled:authorized];
        [[observer nextRoundButton] setEnabled:authorized && playing];
        [[observer callClockButton] setEnabled:authorized && playing];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.elapsed_time_text" options:NSKeyValueObservingOptionInitial block:^(id observer, TBRemoteClockViewController* object, NSDictionary *change) {
        NSString* elapsedTimeText = [[object session] state][@"elapsed_time_text"];
        NSString* fullElapsedTimeText = [NSString localizedStringWithFormat:@"Elapsed Time: %@", elapsedTimeText];
        [[observer elapsedLabel] setText:fullElapsedTimeText];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.clock_text" options:NSKeyValueObservingOptionInitial block:^(id observer, TBRemoteClockViewController* object, NSDictionary *change) {
        [[observer clockLabel] setText:[[object session] state][@"clock_text"]];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.current_round_blinds_text" options:NSKeyValueObservingOptionInitial block:^(id observer, TBRemoteClockViewController* object, NSDictionary *change) {
        [[observer currentBlindsLabel] setText:[[object session] state][@"current_round_blinds_text"]];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.current_round_ante_text" options:NSKeyValueObservingOptionInitial block:^(id observer, TBRemoteClockViewController* object, NSDictionary *change) {
        [[observer currentAnteLabel] setText:[[object session] state][@"current_round_ante_text"]];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.next_round_text" options:NSKeyValueObservingOptionInitial block:^(id observer, TBRemoteClockViewController* object, NSDictionary *change) {
        [[observer nextRoundLabel] setText:[[object session] state][@"next_round_text"]];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.players_left_text" options:NSKeyValueObservingOptionInitial block:^(id observer, TBRemoteClockViewController* object, NSDictionary *change) {
        [[observer playersLeftLabel] setText:[[object session] state][@"players_left_text"]];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.average_stack_text" options:NSKeyValueObservingOptionInitial block:^(id observer, TBRemoteClockViewController* object, NSDictionary *change) {
        [[observer averageStackLabel] setText:[[object session] state][@"average_stack_text"]];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.action_clock_time_remaining" options:NSKeyValueObservingOptionInitial block:^(id observer, TBRemoteClockViewController* object, NSDictionary *change) {
        [observer updateActionClock:[[object session] state][@"action_clock_time_remaining"]];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.background_color" options:NSKeyValueObservingOptionInitial block:^(id observer, TBRemoteClockViewController* object, NSDictionary *change) {
        // Set the background color on the view
        NSString* backgroundColorName = [[object session] state][@"background_color"];
        if(backgroundColorName != nil && ![backgroundColorName isEqualToString:@""]) {
            TBColor* newColor = [TBColor colorWithName:backgroundColorName];
            if(newColor != nil) {
                [[observer view] setBackgroundColor:newColor];
            }
        }

        // get current background color
        TBColor* color = [[observer view] backgroundColor];

        // Set text label appearance to a complementary color.
        if(@available(iOS 9, *)) {
            [[UILabel appearanceWhenContainedInInstancesOfClasses:@[[TBRemoteClockViewController class]]] setTextColor:[color contrastTextColor]];
        } else {
            [[UILabel appearanceWhenContainedIn:[TBRemoteClockViewController class], nil] setTextColor:[color contrastTextColor]];
        }

        // store background mode
        BOOL dark = [color isDark];
        [self setBackgroundIsDark:dark];

        // update status bar
        [self setNeedsStatusBarAppearanceUpdate];

        // invert button images if dark
        [[self previousRoundButton] setImageInverted:dark forState:UIControlStateNormal];
        [[self previousRoundButton] setImageInverted:dark forState:UIControlStateHighlighted];
        [[self pauseResumeButton] setImageInverted:dark forState:UIControlStateNormal];
        [[self pauseResumeButton] setImageInverted:dark forState:UIControlStateHighlighted];
        [[self nextRoundButton] setImageInverted:dark forState:UIControlStateNormal];
        [[self nextRoundButton] setImageInverted:dark forState:UIControlStateHighlighted];
        [[self callClockButton] setImageInverted:dark forState:UIControlStateNormal];
        [[self callClockButton] setImageInverted:dark forState:UIControlStateHighlighted];

        // reload chip table view to invert images
        [[self tableView] reloadData];
    }];

    // register table view cell class
    [[self tableView] registerNib:[UINib nibWithNibName:@"TBChipTableViewCell" bundle:nil] forCellReuseIdentifier:@"ChipCell"];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (UIStatusBarStyle) preferredStatusBarStyle {
    return [self backgroundIsDark] ? UIStatusBarStyleLightContent : UIStatusBarStyleDefault;
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
    if(section == 5) {
        NSArray* availableChips = [[self session] state][@"available_chips"];
        return [availableChips count];
    } else {
        return [super tableView:tableView numberOfRowsInSection:section];
    }
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    if([indexPath section] == 5) {
        // relevent state
        NSArray* availableChips = [[self session] state][@"available_chips"];
        NSDictionary* chip = availableChips[[indexPath row]];

        // create a cell
        TBChipTableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"ChipCell" forIndexPath:indexPath];
        [[cell colorEllipseView] setColor:[TBColor colorWithName:chip[@"color"]]];
        [[cell backgroundImageView] setImageInverted:[self backgroundIsDark]];
        [[cell valueLabel] setText:[chip[@"denomination"] stringValue]];
        return cell;
    } else {
        return [super tableView:tableView cellForRowAtIndexPath:indexPath];
    }
}

#pragma mark UITableViewDelegate

- (UITableViewCellEditingStyle)tableView:(UITableView*)tableView editingStyleForRowAtIndexPath:(NSIndexPath*)indexPath
{
    return UITableViewCellEditingStyleNone;
}

- (CGFloat)tableView:(UITableView*)tableView heightForRowAtIndexPath:(NSIndexPath*)indexPath
{
    // if dynamic section make all rows the same height as row 0
    if([indexPath section] == 5) {
        return [super tableView:tableView heightForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:[indexPath section]]];
    } else {
        return [super tableView:tableView heightForRowAtIndexPath:indexPath];
    }
}

- (NSInteger)tableView:(UITableView*)tableView indentationLevelForRowAtIndexPath:(NSIndexPath*)indexPath
{
    // if dynamic section make all rows the same indentation level as row 0
    if([indexPath section] == 5) {
        return [super tableView:tableView indentationLevelForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:[indexPath section]]];
    } else {
        return [super tableView:tableView indentationLevelForRowAtIndexPath:indexPath];
    }
}

@end
