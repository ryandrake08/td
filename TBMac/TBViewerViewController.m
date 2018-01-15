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
#import "TBColor+ContrastTextColor.h"
#import "TBColor+CSS.h"
#import "TBCurrencyNumberFormatter.h"
#import "TBNotifications.h"
#import "TBSound.h"
#import "TournamentSession.h"

@interface TBViewerViewController () <NSTableViewDelegate>

// The global shared session
@property (strong) TournamentSession* session;

// UI Outlets
@property (weak) IBOutlet NSImageView* backgroundImageView;
@property (weak) IBOutlet NSButton* previousRoundButton;
@property (weak) IBOutlet NSButton* pauseResumeButton;
@property (weak) IBOutlet NSButton* nextRoundButton;
@property (weak) IBOutlet NSButton* callClockButton;
@property (weak) IBOutlet NSTableView* chipsTableView;
@property (weak) IBOutlet NSTableView* resultsTableView;

@property (strong) TBColor* textColor;

// Sounds
@property (nonatomic, strong) TBSound* startSound;
@property (nonatomic, strong) TBSound* nextSound;
@property (nonatomic, strong) TBSound* breakSound;
@property (nonatomic, strong) TBSound* warningSound;
@property (nonatomic, strong) TBSound* rebalanceSound;

// View controllers
@property (strong) TBActionClockViewController* actionClockViewController;

// Array controller for objects managed by this view controller
@property (strong) IBOutlet NSArrayController* resultsArrayController;

@end

@implementation TBViewerViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // alloc colors
    _textColor = [TBColor labelColor];
    
    // alloc sounds
    _startSound = [[TBSound alloc] initWithResource:@"s_start" extension:@"caf"];
    _nextSound = [[TBSound alloc] initWithResource:@"s_next" extension:@"caf"];
    _breakSound = [[TBSound alloc] initWithResource:@"s_break" extension:@"caf"];
    _warningSound = [[TBSound alloc] initWithResource:@"s_warning" extension:@"caf"];
    _rebalanceSound = [[TBSound alloc] initWithResource:@"s_rebalance" extension:@"caf"];

    // update buttons when authorization, connected, or current_blinc_level changes
    [[self KVOController] observe:self keyPaths:@[@"session.state.connected", @"session.state.authorized"] options:NSKeyValueObservingOptionInitial block:^(id observer, TBViewerViewController* object, NSDictionary *change) {
        // update controls
        [self updateTournamentControls];
    }];

    // update action clock when action_clock_time_remaining chantes
    [[self KVOController] observe:self keyPath:@"session.state.action_clock_time_remaining" options:0 block:^(id observer, TBViewerViewController* object, NSDictionary *change) {
        [observer updateActionClock:[[object session] state][@"action_clock_time_remaining"]];
    }];

    // chips tableview reloads itself when available_chips changes
    [[[self chipsTableView] KVOController] observe:self keyPath:@"session.state.available_chips" options:0 action:@selector(reloadData)];

    // register for KVO
    [[self KVOController] observe:self keyPath:@"session.state.current_blind_level" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, TBViewerViewController* object, NSDictionary* change) {
        id old = change[NSKeyValueChangeOldKey];
        id new = change[NSKeyValueChangeNewKey];
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

        // update controls
        [self updateTournamentControls];
    }];

    [[self KVOController] observe:self keyPath:@"session.state.on_break" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, TBViewerViewController* object, NSDictionary* change) {
        id old = change[NSKeyValueChangeOldKey];
        id new = change[NSKeyValueChangeNewKey];
        if(![old isEqualTo:[NSNull null]] && ![new isEqualTo:[NSNull null]]) {
            if([old isEqualTo:@NO] && [new isEqualTo:@YES]) {
                // break NO to YES
                [[self breakSound] play];
            }
        }
    }];

    [[self KVOController] observe:self keyPaths:@[@"session.state.time_remaining",@"session.state.break_time_remaining"] options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld block:^(id observer, TBViewerViewController* object, NSDictionary* change) {
        id old = change[NSKeyValueChangeOldKey];
        id new = change[NSKeyValueChangeNewKey];
        if(![old isEqualTo:[NSNull null]] && ![new isEqualTo:[NSNull null]]) {
            if([old integerValue] > kAudioWarningTime && [new integerValue] <= kAudioWarningTime && [new integerValue] != 0) {
                // time crosses kAudioWarningTime
                [[self warningSound] play];
            }
        }
    }];

    [[self KVOController] observe:self keyPath:@"session.state.background_color" options:NSKeyValueObservingOptionInitial block:^(id observer, TBViewerViewController* object, NSDictionary *change) {
        // Set the background color on the view
        NSString* backgroundColorName = [[object session] state][@"background_color"];
        if(backgroundColorName != nil) {
            TBColor* color = [TBColor colorWithName:backgroundColorName];
            [[object view] setBackgroundColor:color];

            // All text fields in view are bound to this color. Set once, set for all
            [object setTextColor:[color contrastTextColor]];

            // Invert button images if dark
            BOOL dark = [color isDark];
#if 0
            [[self previousRoundButton] setImageInverted:dark forState:UIControlStateNormal];
            [[self previousRoundButton] setImageInverted:dark forState:UIControlStateHighlighted];
            [[self pauseResumeButton] setImageInverted:dark forState:UIControlStateNormal];
            [[self pauseResumeButton] setImageInverted:dark forState:UIControlStateHighlighted];
            [[self nextRoundButton] setImageInverted:dark forState:UIControlStateNormal];
            [[self nextRoundButton] setImageInverted:dark forState:UIControlStateHighlighted];
            [[self callClockButton] setImageInverted:dark forState:UIControlStateNormal];
            [[self callClockButton] setImageInverted:dark forState:UIControlStateHighlighted];
#endif
        }
    }];

    // register for movement notification
    [[NSNotificationCenter defaultCenter] addObserverForName:kMovementsUpdatedNotification object:nil queue:nil usingBlock:^(NSNotification* note) {
        [[self rebalanceSound] play];
    }];

    // set up sort descriptor for results
    NSSortDescriptor* placeSort = [[NSSortDescriptor alloc] initWithKey:@"place" ascending:YES];
    [[self resultsArrayController] setSortDescriptors:@[placeSort]];
}

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    if([[segue identifier] isEqualToString:@"presentActionClockView"]) {
        [self setActionClockViewController:[segue destinationController]];
        [[self actionClockViewController] setRepresentedObject:[self session]];
    }
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // set session
    [self setSession:representedObject];

    // also set for containers
    [[self actionClockViewController] setRepresentedObject:representedObject];
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView*)aTableView viewForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    NSTableCellView* result = [aTableView makeViewWithIdentifier:aTableColumn.identifier owner:self];
    if([[aTableColumn identifier] isEqualToString:@"Payout"]) {
        if([[result textField] formatter] == nil) {
            [[result textField] setFormatter:[[TBCurrencyNumberFormatter alloc] init]];
        }
        NSDictionary* object = [[[self resultsArrayController] arrangedObjects] objectAtIndex:rowIndex];
        [[[result textField] formatter] setCurrencyCode:object[@"payout_currency"]];
    } else if([[aTableColumn identifier] isEqualToString:@"Chip"]) {
        [[result imageView] bind:@"color" toObject:result withKeyPath:@"objectValue.color" options:@{NSValueTransformerNameBindingOption:@"TBColorValueTransformer"}];
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
    BOOL connected = [[[self session] state][@"connected"] boolValue];
    BOOL authorized = [[[self session] state][@"authorized"] boolValue];
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
