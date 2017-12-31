//
//  TBViewerViewController.m
//  td
//
//  Created by Ryan Drake on 6/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBViewerViewController.h"
#import "TBActionClockViewController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TBSoundPlayer.h"
#import "NSObject+FBKVOController.h"

@interface TBViewerViewController () <NSTableViewDelegate>

// UI elements
@property (weak) IBOutlet NSImageView* backgroundImageView;
@property (weak) IBOutlet NSButton* previousRoundButton;
@property (weak) IBOutlet NSButton* pauseResumeButton;
@property (weak) IBOutlet NSButton* nextRoundButton;
@property (weak) IBOutlet NSButton* callClockButton;
@property (weak) IBOutlet NSTableView* chipsTableView;
@property (weak) IBOutlet NSTableView* resultsTableView;

// Sound player
@property (strong) IBOutlet TBSoundPlayer* soundPlayer;

// Array controller for objects managed by this view controller
@property (strong) IBOutlet NSArrayController* resultsArrayController;

@end

@implementation TBViewerViewController

- (void)viewDidLoad {
    if([NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [super viewDidLoad];
    }

    // update buttons when authorization, connected, or current_blinc_level changes
    [[self KVOController] observe:self keyPaths:@[@"session.state.connected", @"session.state.authorized", @"session.state.current_blind_level"] options:NSKeyValueObservingOptionInitial block:^(id observer, TBViewerViewController* object, NSDictionary *change) {
        BOOL authorized = [[[object session] state][@"connected"] boolValue] && [[[object session] state][@"authorized"] boolValue];
        BOOL playing = [[[object session] state][@"current_blind_level"] unsignedIntegerValue] != 0;
        [[observer previousRoundButton] setEnabled:authorized && playing];
        [[observer pauseResumeButton] setEnabled:authorized];
        [[observer nextRoundButton] setEnabled:authorized && playing];
        [[observer callClockButton] setEnabled:authorized && playing];
    }];

    // update action clock when action_clock_time_remaining chantes
    [[self KVOController] observe:self keyPath:@"session.state.action_clock_time_remaining" options:0 block:^(id observer, TBViewerViewController* object, NSDictionary *change) {
        [observer updateActionClock:[[object session] state][@"action_clock_time_remaining"]];
    }];

    // chips tableview reloads itself when available_chips changes
    [[[self chipsTableView] KVOController] observe:self keyPath:@"session.state.available_chips" options:0 action:@selector(reloadData)];

    // set up sort descriptor for results
    NSSortDescriptor* placeSort = [[NSSortDescriptor alloc] initWithKey:@"place" ascending:YES];
    [[self resultsArrayController] setSortDescriptors:@[placeSort]];

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
    if([[segue identifier] isEqualToString:@"presentActionClockView"]) {
        TBActionClockViewController* vc =  [segue destinationController];
        [vc setSession:[self session]];
    }
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
