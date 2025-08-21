//
//  TBViewerViewController.m
//  td
//
//  Created by Ryan Drake on 6/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBViewerViewController.h"
#import "NSObject+FBKVOController.h"
#import "TBActionClockViewController.h"
#import "TBChipTableCellView.h"
#import "TBColor+ContrastTextColor.h"
#import "TBColor+CSS.h"
#import "TBCurrencyNumberFormatter.h"
#import "TBEllipseView.h"
#import "TBInvertableButton_macOS.h"
#import "TBInvertableImageView.h"
#import "TBNotifications.h"
#import "TBSoundPlayer.h"
#import "TournamentSession.h"

@interface TBViewerViewController () <NSTableViewDelegate>

// Sound player
@property (nonatomic, strong) TBSoundPlayer* soundPlayer;

// UI Outlets
@property (nonatomic, weak) IBOutlet NSImageView* backgroundImageView;
@property (nonatomic, weak) IBOutlet TBInvertableButton* previousRoundButton;
@property (nonatomic, weak) IBOutlet TBInvertableButton* pauseResumeButton;
@property (nonatomic, weak) IBOutlet TBInvertableButton* nextRoundButton;
@property (nonatomic, weak) IBOutlet TBInvertableButton* callClockButton;
@property (nonatomic, weak) IBOutlet NSTableView* chipsTableView;
@property (nonatomic, weak) IBOutlet NSTableView* resultsTableView;

// Background color
@property (nonatomic, strong) TBColor* textColor;
@property (nonatomic, assign) BOOL backgroundIsDark;

// View controllers
@property (nonatomic, strong) TBActionClockViewController* actionClockViewController;

// Array controller for objects managed by this view controller
@property (nonatomic, strong) IBOutlet NSArrayController* resultsArrayController;

@end

@implementation TBViewerViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // alloc sound player
    _soundPlayer = [[TBSoundPlayer alloc] init];

    // alloc colors
    _textColor = [TBColor labelColor];

    // bind button state
    [[self previousRoundButton] bind:@"imageInverted" toObject:self withKeyPath:@"backgroundIsDark" options:nil];
    [[self previousRoundButton] bind:@"alternateImageInverted" toObject:self withKeyPath:@"backgroundIsDark" options:nil];
    [[self pauseResumeButton] bind:@"imageInverted" toObject:self withKeyPath:@"backgroundIsDark" options:nil];
    [[self pauseResumeButton] bind:@"alternateImageInverted" toObject:self withKeyPath:@"backgroundIsDark" options:nil];
    [[self nextRoundButton] bind:@"imageInverted" toObject:self withKeyPath:@"backgroundIsDark" options:nil];
    [[self nextRoundButton] bind:@"alternateImageInverted" toObject:self withKeyPath:@"backgroundIsDark" options:nil];
    [[self callClockButton] bind:@"imageInverted" toObject:self withKeyPath:@"backgroundIsDark" options:nil];
    [[self callClockButton] bind:@"alternateImageInverted" toObject:self withKeyPath:@"backgroundIsDark" options:nil];

    // update window title when tournament name changes
    [[self KVOController] observe:self keyPath:@"session.state.name" options:NSKeyValueObservingOptionInitial block:^(id observer, TBViewerViewController* object, NSDictionary *change) {
        NSString* tournamentName = [[object session] state][@"name"];
        if(tournamentName == nil) {
            [[[self view] window] setTitle:NSLocalizedString(@"Tournament Display", nil)];
        } else {
            [[[self view] window] setTitle:[NSString localizedStringWithFormat:@"Display: %@", tournamentName]];
        }
    }];

    // update buttons when authorization, connected, or current_blind_level changes
    [[self KVOController] observe:self keyPaths:@[@"session.connected", @"session.authorized", @"session.state.current_blind_level"] options:NSKeyValueObservingOptionInitial block:^(id observer, TBViewerViewController* object, NSDictionary *change) {
        // update controls
        [self updateTournamentControls];
    }];

    // update action clock when action_clock_time_remaining changes
    [[self KVOController] observe:self keyPath:@"session.state.action_clock_time_remaining" options:0 block:^(id observer, TBViewerViewController* object, NSDictionary *change) {
        [observer updateActionClock:[[object session] state][@"action_clock_time_remaining"]];
    }];

    // chips tableview reloads itself when available_chips changes
    [[[self chipsTableView] KVOController] observe:self keyPath:@"session.state.available_chips" options:0 action:@selector(reloadData)];

    // set background color when it changes
    [[self KVOController] observe:self keyPath:@"session.state.background_color" options:NSKeyValueObservingOptionInitial block:^(id observer, TBViewerViewController* object, NSDictionary *change) {
        // Set the background color on the view
        NSString* backgroundColorName = [[object session] state][@"background_color"];
        if(backgroundColorName != nil && ![backgroundColorName isEqualToString:@""]) {
            TBColor* color = [TBColor colorWithName:backgroundColorName];
            [[object view] setBackgroundColor:color];

            // All text fields in view are bound to this color. Set once, set for all
            [object setTextColor:[color contrastTextColor]];

            // store background mode
            BOOL dark = [color isDark];
            [self setBackgroundIsDark:dark];
        }
    }];

    // set up sort descriptor for results
    NSSortDescriptor* placeSort = [[NSSortDescriptor alloc] initWithKey:@"place" ascending:YES];
    [[self resultsArrayController] setSortDescriptors:@[placeSort]];
}

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    if([[segue identifier] isEqualToString:@"presentActionClockView"]) {
        [self setActionClockViewController:[segue destinationController]];
        [[self actionClockViewController] setSession:[self session]];
    }
}

- (void)setSession:(TournamentSession*)session {
    // set session
    _session = session;

    // also set for containers
    [[self actionClockViewController] setSession:[self session]];

    // also set for sound player
    [[self soundPlayer] setSession:[self session]];
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView*)aTableView viewForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    NSTableCellView* result = [aTableView makeViewWithIdentifier:aTableColumn.identifier owner:self];
    if([[aTableColumn identifier] isEqualToString:@"Payout"]) {
        NSString* payoutCurrency = [[self session] state][@"payout_currency"];
        [[[result textField] formatter] setCurrencyCode:payoutCurrency];
    } else if([[aTableColumn identifier] isEqualToString:@"Chip"]) {
        TBChipTableCellView* chipTableCellView = (TBChipTableCellView*)result;
        [[chipTableCellView colorEllipseView] bind:@"color" toObject:chipTableCellView withKeyPath:@"objectValue.color" options:@{NSValueTransformerNameBindingOption:@"TBColorValueTransformer"}];
        [[chipTableCellView backgroundImageView] bind:@"imageInverted" toObject:self withKeyPath:@"backgroundIsDark" options:nil];
    } else {
        NSLog(@"viewForTableColumn: %@", [aTableColumn identifier]);
    }
    return result;
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

#pragma mark Enable/disable buttons
- (void)updateTournamentControls {
    BOOL connected = [[self session] connected];
    BOOL authorized = [[self session] authorized];
    BOOL playing = [[[self session] state][@"current_blind_level"] unsignedIntegerValue] != 0;
    [[self previousRoundButton] setEnabled:connected && authorized && playing];
    [[self pauseResumeButton] setEnabled:connected && authorized];
    [[self nextRoundButton] setEnabled:connected && authorized && playing];
    [[self callClockButton] setEnabled:connected && authorized && playing];
}

#pragma mark Actions

- (IBAction)previousRoundTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGame];
    }
}

- (IBAction)nextRoundTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(NSButton*)sender {
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

@end
