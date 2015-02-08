//
//  TBRemoteClockViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRemoteClockViewController.h"
#import "TournamentKit/TournamentKit.h"
#import "TBActionClockView.h"
#import "TBAppDelegate.h"

#import "NSObject+FBKVOController.h"

@interface TBRemoteClockViewController () <TBActionClockDelegate>

@property (nonatomic) TournamentSession* session;

@property (nonatomic) NSNumberFormatter* decimalFormatter;

@property (nonatomic, weak) IBOutlet UILabel* elapsedLabel;
@property (nonatomic, weak) IBOutlet UILabel* clockLabel;
@property (nonatomic, weak) IBOutlet UILabel* currentRoundLabel;
@property (nonatomic, weak) IBOutlet UILabel* nextRoundLabel;
@property (nonatomic, weak) IBOutlet UILabel* playersLeftLabel;
@property (nonatomic, weak) IBOutlet UILabel* averageStackLabel;
@property (nonatomic, weak) IBOutlet UIButton* previousRoundButton;
@property (nonatomic, weak) IBOutlet UIButton* pauseResumeButton;
@property (nonatomic, weak) IBOutlet UIButton* nextRoundButton;
@property (nonatomic, weak) IBOutlet UIButton* callClockButton;
@property (nonatomic, weak) IBOutlet TBActionClockView* actionClockView;


- (IBAction)previousRoundTapped:(UIButton*)sender;
- (IBAction)pauseResumeTapped:(UIButton*)sender;
- (IBAction)nextRoundTapped:(UIButton*)sender;
- (IBAction)callClockTapped:(UIButton*)sender;

@end

@implementation TBRemoteClockViewController

#define kActionClockRequestTime 60000

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // Make formatter
    _decimalFormatter = [[NSNumberFormatter alloc] init];
    [[self decimalFormatter] setNumberStyle:NSNumberFormatterDecimalStyle];

    // register for KVO
    [[self KVOController] observe:[self session] keyPaths:@[@"isConnected", @"isAuthorized"] options:0 block:^(id observer, id object, NSDictionary *change) {
        [observer updateButtons];
    }];

    [[self KVOController] observe:[self session] keyPath:@"blindLevels" options:0 block:^(id observer, id object, NSDictionary *change) {
        [observer updateBlinds];
    }];

    [[self KVOController] observe:[self session] keyPath:@"currentBlindLevel" options:0 block:^(id observer, id object, NSDictionary *change) {
        [observer updateButtons];
        [observer updateBlinds];
    }];

    [[self KVOController] observe:[self session] keyPaths:@[@"isRunning", @"timeRemaining", @"breakTimeRemaining"] options:0 block:^(id observer, id object, NSDictionary *change) {
        [observer updateClock];
        [observer updateBlinds];
    }];

    [[self KVOController] observe:[self session] keyPath:@"seats" options:0 block:^(id observer, id object, NSDictionary *change) {
        [observer updatePlayers];
        [observer updateAverageStack];
    }];

    [[self KVOController] observe:[self session] keyPath:@"totalChips" options:0 block:^(id observer, id object, NSDictionary *change) {
        [observer updateAverageStack];
    }];

    [[self KVOController] observe:[self session] keyPath:@"actionClockTimeRemaining" options:0 block:^(id observer, id object, NSDictionary *change) {
        [observer updateActionClock];
    }];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];

    // set up initial values for buttons and labels
    [self updateButtons];
    [self updateClock];
    [self updateBlinds];
    [self updatePlayers];
    [self updateAverageStack];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark Formatters

- (NSString*)formatBlindLevel:(NSDictionary*)level {
    NSNumber* bigBlind = level[@"big_blind"];
    NSNumber* littleBlind = level[@"little_blind"];
    NSNumber* ante = level[@"ante"];

    if([ante unsignedIntegerValue] > 0) {
        return [NSString localizedStringWithFormat:@"%@/%@ A:%@",
                [[self decimalFormatter] stringFromNumber:littleBlind],
                [[self decimalFormatter] stringFromNumber:bigBlind],
                [[self decimalFormatter] stringFromNumber:ante]];
    } else {
        return [NSString localizedStringWithFormat:@"%@/%@",
                [[self decimalFormatter] stringFromNumber:littleBlind],
                [[self decimalFormatter] stringFromNumber:bigBlind]];
    }
}

- (NSString*)formatDuration:(NSUInteger)duration {
    if(duration < 60000) {
        // SS.MSS
        unsigned long s = duration / 1000 % 60;
        unsigned long ms = duration % 1000;
        return [NSString stringWithFormat:@"%lu.%03lu", s, ms];
    } else if(duration < 3600000) {
        // MM:SS
        unsigned long m = duration / 60000;
        unsigned long s = duration / 1000 % 60;
        return [NSString stringWithFormat:@"%lu:%02lu", m, s];
    } else {
        // HH:MM:SS
        unsigned long h = duration / 3600000;
        unsigned long m = duration / 60000 % 60;
        unsigned long s = duration / 1000 % 60;
        return [NSString stringWithFormat:@"%lu:%02lu:%02lu", h, m, s];
    }
}

#pragma mark Update

- (void)updateButtons {
    BOOL connected = [[self session] isConnected];
    BOOL authorized = [[self session] isAuthorized];
    if(connected && authorized) {
        [[self previousRoundButton] setHidden:NO];
        [[self pauseResumeButton] setHidden:NO];
        [[self nextRoundButton] setHidden:NO];
        [[self callClockButton] setHidden:NO];
    } else {
        [[self previousRoundButton] setHidden:YES];
        [[self pauseResumeButton] setHidden:YES];
        [[self nextRoundButton] setHidden:YES];
        [[self callClockButton] setHidden:YES];
    }

    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel == 0) {
        [[self previousRoundButton] setEnabled:NO];
        [[self pauseResumeButton] setEnabled:YES];
        [[self nextRoundButton] setEnabled:NO];
        [[self callClockButton] setEnabled:NO];
    } else {
        [[self previousRoundButton] setEnabled:YES];
        [[self pauseResumeButton] setEnabled:YES];
        [[self nextRoundButton] setEnabled:YES];
        [[self callClockButton] setEnabled:YES];
    }
}

- (void)updateClock {
    BOOL running = [[[self session] isRunning] boolValue];
    NSUInteger timeRemaining = [[[self session] timeRemaining] unsignedIntegerValue];
    NSUInteger breakTimeRemaining = [[[self session] breakTimeRemaining] unsignedIntegerValue];

    if(running) {
        if(timeRemaining == 0 && breakTimeRemaining != 0) {
            // on break
            [[self clockLabel] setText:[self formatDuration:breakTimeRemaining]];
        } else {
            [[self clockLabel] setText:[self formatDuration:timeRemaining]];
        }
    } else {
        [[self clockLabel] setText:NSLocalizedString(@"PAUSED", nil)];
    }
}

- (void)updateBlinds {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    NSArray* blindLevels = [[self session] blindLevels];
    NSUInteger timeRemaining = [[[self session] timeRemaining] unsignedIntegerValue];
    NSUInteger breakTimeRemaining = [[[self session] breakTimeRemaining] unsignedIntegerValue];

    if(currentBlindLevel == 0) {
        [[self currentRoundLabel] setText:NSLocalizedString(@"PLANNING", nil)];
    } else {
        if(timeRemaining == 0 && breakTimeRemaining != 0) {
            [[self currentRoundLabel] setText:NSLocalizedString(@"BREAK", nil)];
        } else if(currentBlindLevel < [blindLevels count]) {
            NSDictionary* thisBlindLevel = blindLevels[currentBlindLevel];
            [[self currentRoundLabel] setText:[self formatBlindLevel:thisBlindLevel]];
        }

        if(currentBlindLevel+1 < [blindLevels count]) {
            NSDictionary* nextBlindLevel = blindLevels[currentBlindLevel+1];
            [[self nextRoundLabel] setText:[self formatBlindLevel:nextBlindLevel]];
        }
    }
}

- (void)updatePlayers {
    NSArray* seats = [[self session] seats];

    if([seats count] > 0) {
        NSNumber* numSeats = [NSNumber numberWithUnsignedInteger:[seats count]];
        NSString* numSeatsText = [[self decimalFormatter] stringFromNumber:numSeats];
        [[self playersLeftLabel] setText:numSeatsText];
    } else {
        [[self playersLeftLabel] setText:@"-"];
    }
}

- (void)updateAverageStack {
    NSArray* seats = [[self session] seats];
    NSUInteger totalChips = [[[self session] totalChips] unsignedIntegerValue];

    if([seats count] > 0) {
        NSNumber* avgChips = [NSNumber numberWithUnsignedInteger:totalChips / [seats count]];
        NSString* avgChipsText = [[self decimalFormatter] stringFromNumber:avgChips];
        [[self averageStackLabel] setText:avgChipsText];
    } else {
        [[self averageStackLabel] setText:@"-"];
    }
}

- (void)updateActionClock {
    NSUInteger actionClockTimeRemaining = [[[self session] actionClockTimeRemaining] unsignedIntegerValue];
    if(actionClockTimeRemaining == 0) {
        [[self actionClockView] setHidden:YES];
    } else {
        [[self actionClockView] setHidden:NO];
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }
}

#pragma mark Actions

- (IBAction)previousRoundTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGameAt:nil];
    }
}

- (IBAction)nextRoundTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        NSUInteger remaining = [[[self session] actionClockTimeRemaining] unsignedIntegerValue];
        if(remaining == 0) {
            [[self session] setActionClock:[NSNumber numberWithUnsignedInteger:kActionClockRequestTime]];
        } else {
            [[self session] setActionClock:nil];
        }
    }
}

# pragma mark TBActionClockViewDelegate

- (CGFloat)analogClock:(TBActionClockView*)clock graduationLengthForIndex:(NSInteger)index {
    return index % 5 == 0 ? 10.0 : 5.0;
}

@end
