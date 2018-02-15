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

// The global shared session
@property (strong) TournamentSession* session;

// UI Outlets
@property (weak) IBOutlet NSView* leftPaneView;
@property (weak) IBOutlet NSView* rightPaneView;
@property (weak) IBOutlet NSView* centerPaneView;

// View Controllers
@property (strong) TBSeatingViewController* seatingViewController;
@property (strong) TBPlayersViewController* playersViewController;
@property (strong) TBResultsViewController* resultsViewController;

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

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // set session
    [self setSession:representedObject];

    // also set for containers
    [[self seatingViewController] setRepresentedObject:representedObject];
    [[self playersViewController] setRepresentedObject:representedObject];
    [[self resultsViewController] setRepresentedObject:representedObject];
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

- (IBAction)planSeatingTapped:(id)sender {
    if([[[self session] state][@"seats"] count] > 0 || [[[self session] state][@"buyins"] count] > 0) {
        // alert because this is a very destructive action
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSWarningAlertStyle];
        [alert addButtonWithTitle:NSLocalizedString(@"Plan", nil)];
        [alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)];
        [alert setMessageText:NSLocalizedString(@"Plan Seating", nil)];

        // display a different message if the game is running
        BOOL playing = [[[self session] state][@"current_blind_level"] unsignedIntegerValue] != 0;
        if(playing) {
            [alert setInformativeText:NSLocalizedString(@"This will end the current tournament immediately, then clear any existing seats and buy-ins.", nil)];
        } else {
            [alert setInformativeText:NSLocalizedString(@"This will clear any existing seats and buy-ins.", nil)];
        }

        // present and only perform setup if confirmed by user
        if([alert runModal] == NSAlertFirstButtonReturn) {
            [(TBMacDocument*)[self document] planSeating];
        }
    } else {
        // no warning
        [(TBMacDocument*)[self document] planSeating];
    }
}

- (IBAction)rebalanceTapped:(id)sender {
    [[self session] rebalanceSeatingWithBlock:^(NSArray* movements) {
        if([movements count] > 0) {
            [[NSNotificationCenter defaultCenter] postNotificationName:kMovementsUpdatedNotification object:movements];
            [[[[self view] window] windowController] performSegueWithIdentifier:@"presentMovementView" sender:sender];
        }
    }];
}

- (IBAction)quickStartTapped:(id)sender {
    if([[[self session] state][@"seats"] count] > 0 || [[[self session] state][@"buyins"] count] > 0) {
        // alert because this is a very destructive action
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSWarningAlertStyle];
        [alert addButtonWithTitle:NSLocalizedString(@"Setup", nil)];
        [alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)];
        [alert setMessageText:NSLocalizedString(@"Quick Setup", nil)];

        // display a different message if the game is running
        BOOL playing = [[[self session] state][@"current_blind_level"] unsignedIntegerValue] != 0;
        if(playing) {
            [alert setInformativeText:NSLocalizedString(@"Quick Setup will end the current tournament immediately, then re-seat and buy in all players.", nil)];
        } else {
            [alert setInformativeText:NSLocalizedString(@"Quick Setup will clear any existing seats and buy-ins, then re-seat and buy in all players.", nil)];
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
