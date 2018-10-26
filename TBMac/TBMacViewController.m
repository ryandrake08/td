//
//  TBMacViewController.m
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBMacViewController.h"
#import "TBMacDocument.h"
#import "TBNotifications.h"
#import "TBSeatingViewController.h"
#import "TBResultsViewController.h"
#import "TBPlayersViewController.h"
#import "TournamentSession.h"

@interface TBMacViewController ()

// UI Outlets
@property (nonatomic, weak) IBOutlet NSView* leftPaneView;
@property (nonatomic, weak) IBOutlet NSView* rightPaneView;
@property (nonatomic, weak) IBOutlet NSView* centerPaneView;

// View Controllers
@property (nonatomic, strong) TBSeatingViewController* seatingViewController;
@property (nonatomic, strong) TBPlayersViewController* playersViewController;
@property (nonatomic, strong) TBResultsViewController* resultsViewController;

@end

@implementation TBMacViewController

- (TBMacDocument*)document {
    return [[[[self view] window] windowController] document];
}

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    // reference the container view controllers
    if([[segue identifier] isEqualToString:@"presentSeatingView"]) {
        [self setSeatingViewController:[segue destinationController]];
    } else if([[segue identifier] isEqualToString:@"presentPlayersView"]) {
        [self setPlayersViewController:[segue destinationController]];
    } else if([[segue identifier] isEqualToString:@"presentResultsView"]) {
        [self setResultsViewController:[segue destinationController]];
    }

    // once we have both of these, set seating vc as delegate of players vc
    if([self playersViewController] && [self seatingViewController]) {
        [[self playersViewController] setDelegate:[self seatingViewController]];
    }
}

- (void)setSession:(TournamentSession*)session {
    // set session
    _session = session;

    // also set for containers
    [[self seatingViewController] setSession:[self session]];
    [[self playersViewController] setSession:[self session]];
    [[self resultsViewController] setSession:[self session]];
}

#pragma mark Attributes

- (NSView*)printableView {
    return [[self seatingViewController] view];
}

#pragma mark Actions

- (IBAction)exportResults:(id)sender {
    NSSavePanel* savePanel = [NSSavePanel savePanel];
    [savePanel setShowsTagField:NO];
    [savePanel setTitle:NSLocalizedString(@"Export Results...", @"Export results to a file")];
    [savePanel setAllowedFileTypes:@[@"CSV"]];
    [savePanel beginSheetModalForWindow:[[self view] window] completionHandler:^(NSInteger result) {
        if(result == NSFileHandlingPanelOKButton) {
            [[self document] saveToURL:[savePanel URL] ofType:@"CSV" forSaveOperation:NSSaveToOperation completionHandler:^(NSError* errorOrNil) {
                NSLog(@"%@", errorOrNil);
            }];
        }
    }];
}

- (IBAction)tournamentNameWasChanged:(NSTextField*)sender {
    // resign first responder
    [[sender window] selectNextKeyView:self];

    // configure session and replace current configuration
    [(TBMacDocument*)[self document] addConfiguration:@{@"name":[sender stringValue]}];
}

- (IBAction)endGameTapped:(id)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] stopGame];
    }
}

- (IBAction)previousRoundTapped:(id)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(id)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGame];
    }
}

- (IBAction)nextRoundTapped:(id)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(id)sender {
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

- (IBAction)quickStartTapped:(id)sender {
    if([[[self session] state][@"seats"] count] > 0 || [[[self session] state][@"buyins"] count] > 0) {
        // alert because this is a very destructive action
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSWarningAlertStyle];
        [alert addButtonWithTitle:NSLocalizedString(@"Setup", nil)];
        [alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)];
        [alert setMessageText:NSLocalizedString(@"Quick Start", nil)];

        // display a different message if the game is running
        BOOL playing = [[[self session] state][@"current_blind_level"] unsignedIntegerValue] != 0;
        if(playing) {
            [alert setInformativeText:NSLocalizedString(@"Quick Start will end the current tournament immediately, then re-seat and buy in all players.", nil)];
        } else {
            [alert setInformativeText:NSLocalizedString(@"Quick Start will clear any existing seats and buy-ins, then re-seat and buy in all players.", nil)];
        }

        // present and only perform setup if confirmed by user
        if([alert runModal] == NSAlertFirstButtonReturn) {
            [[self session] quickSetupWithBlock:nil];
        }
    } else {
        // no warning
        [[self session] quickSetupWithBlock:nil];
    }
}

@end
