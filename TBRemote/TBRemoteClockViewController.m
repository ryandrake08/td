//
//  TBRemoteClockViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRemoteClockViewController.h"
#import "TournamentKit_ios/TournamentKit.h"
#import "TBActionClockView.h"

@interface TBRemoteClockViewController () <TBActionClockDelegate>

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

    // Make formatter
    _decimalFormatter = [[NSNumberFormatter alloc] init];
    [[self decimalFormatter] setNumberStyle:NSNumberFormatterDecimalStyle];

    // register for KVO
    [[TournamentSession sharedSession] addObserver:self forKeyPath:NSStringFromSelector(@selector(isConnected)) options:0 context:NULL];
    [[TournamentSession sharedSession] addObserver:self forKeyPath:NSStringFromSelector(@selector(isAuthorized)) options:0 context:NULL];
    [[TournamentSession sharedSession] addObserver:self forKeyPath:NSStringFromSelector(@selector(blindLevels)) options:0 context:NULL];
    [[TournamentSession sharedSession] addObserver:self forKeyPath:NSStringFromSelector(@selector(isRunning)) options:0 context:NULL];
    [[TournamentSession sharedSession] addObserver:self forKeyPath:NSStringFromSelector(@selector(currentBlindLevel)) options:0 context:NULL];
    [[TournamentSession sharedSession] addObserver:self forKeyPath:NSStringFromSelector(@selector(timeRemaining)) options:0 context:NULL];
    [[TournamentSession sharedSession] addObserver:self forKeyPath:NSStringFromSelector(@selector(breakTimeRemaining)) options:0 context:NULL];
    [[TournamentSession sharedSession] addObserver:self forKeyPath:NSStringFromSelector(@selector(totalChips)) options:0 context:NULL];
    [[TournamentSession sharedSession] addObserver:self forKeyPath:NSStringFromSelector(@selector(seats)) options:0 context:NULL];
    [[TournamentSession sharedSession] addObserver:self forKeyPath:NSStringFromSelector(@selector(actionClockTimeRemaining)) options:0 context:NULL];
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

- (void)dealloc {
    // unregister for KVO
    [[TournamentSession sharedSession] removeObserver:self forKeyPath:NSStringFromSelector(@selector(isConnected))];
    [[TournamentSession sharedSession] removeObserver:self forKeyPath:NSStringFromSelector(@selector(isAuthorized))];
    [[TournamentSession sharedSession] removeObserver:self forKeyPath:NSStringFromSelector(@selector(blindLevels))];
    [[TournamentSession sharedSession] removeObserver:self forKeyPath:NSStringFromSelector(@selector(isRunning))];
    [[TournamentSession sharedSession] removeObserver:self forKeyPath:NSStringFromSelector(@selector(currentBlindLevel))];
    [[TournamentSession sharedSession] removeObserver:self forKeyPath:NSStringFromSelector(@selector(timeRemaining))];
    [[TournamentSession sharedSession] removeObserver:self forKeyPath:NSStringFromSelector(@selector(breakTimeRemaining))];
    [[TournamentSession sharedSession] removeObserver:self forKeyPath:NSStringFromSelector(@selector(totalChips))];
    [[TournamentSession sharedSession] removeObserver:self forKeyPath:NSStringFromSelector(@selector(seats))];
    [[TournamentSession sharedSession] removeObserver:self forKeyPath:NSStringFromSelector(@selector(actionClockTimeRemaining))];
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
        NSUInteger s = duration / 1000 % 60;
        NSUInteger ms = duration % 1000;
        return [NSString stringWithFormat:@"%lu.%03lu", s, ms];
    } else if(duration < 3600000) {
        // MM:SS
        NSUInteger m = duration / 60000;
        NSUInteger s = duration / 1000 % 60;
        return [NSString stringWithFormat:@"%lu:%02lu", m, s];
    } else {
        // HH:MM:SS
        NSUInteger h = duration / 3600000;
        NSUInteger m = duration / 60000 % 60;
        NSUInteger s = duration / 1000 % 60;
        return [NSString stringWithFormat:@"%lu:%02lu:%02lu", h, m, s];
    }
}

#pragma mark Update

- (void)updateButtons {
    BOOL connected = [[TournamentSession sharedSession] isConnected];
    BOOL authorized = [[TournamentSession sharedSession] isAuthorized];
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

    NSUInteger currentBlindLevel = [[[TournamentSession sharedSession] currentBlindLevel] unsignedIntegerValue];
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
    BOOL running = [[[TournamentSession sharedSession] isRunning] boolValue];
    NSUInteger timeRemaining = [[[TournamentSession sharedSession] timeRemaining] unsignedIntegerValue];
    NSUInteger breakTimeRemaining = [[[TournamentSession sharedSession] breakTimeRemaining] unsignedIntegerValue];

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
    NSUInteger currentBlindLevel = [[[TournamentSession sharedSession] currentBlindLevel] unsignedIntegerValue];
    NSArray* blindLevels = [[TournamentSession sharedSession] blindLevels];
    NSUInteger timeRemaining = [[[TournamentSession sharedSession] timeRemaining] unsignedIntegerValue];
    NSUInteger breakTimeRemaining = [[[TournamentSession sharedSession] breakTimeRemaining] unsignedIntegerValue];

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
    NSArray* seats = [[TournamentSession sharedSession] seats];

    if([seats count] > 0) {
        NSNumber* numSeats = [NSNumber numberWithUnsignedInteger:[seats count]];
        NSString* numSeatsText = [[self decimalFormatter] stringFromNumber:numSeats];
        [[self playersLeftLabel] setText:numSeatsText];
    } else {
        [[self playersLeftLabel] setText:@"-"];
    }
}

- (void)updateAverageStack {
    NSArray* seats = [[TournamentSession sharedSession] seats];
    NSUInteger totalChips = [[[TournamentSession sharedSession] totalChips] unsignedIntegerValue];

    if([seats count] > 0) {
        NSNumber* avgChips = [NSNumber numberWithUnsignedInteger:totalChips / [seats count]];
        NSString* avgChipsText = [[self decimalFormatter] stringFromNumber:avgChips];
        [[self averageStackLabel] setText:avgChipsText];
    } else {
        [[self averageStackLabel] setText:@"-"];
    }
}

- (void)updateActionClock {
    NSUInteger actionClockTimeRemaining = [[[TournamentSession sharedSession] actionClockTimeRemaining] unsignedIntegerValue];
    if(actionClockTimeRemaining == 0) {
        [[self actionClockView] setHidden:YES];
    } else {
        [[self actionClockView] setHidden:NO];
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }
}

#pragma mark KVO

- (void)observeValueForKeyPath:(NSString*)keyPath ofObject:(id)session change:(NSDictionary*)change context:(void*)context {
    if ([session isKindOfClass:[TournamentSession class]]) {
        if([keyPath isEqualToString:NSStringFromSelector(@selector(isRunning))] ||
           [keyPath isEqualToString:NSStringFromSelector(@selector(timeRemaining))] ||
           [keyPath isEqualToString:NSStringFromSelector(@selector(breakTimeRemaining))]) {
            [self updateClock];
            [self updateBlinds];
        }

        if([keyPath isEqualToString:NSStringFromSelector(@selector(blindLevels))]) {
            [self updateBlinds];
        }

        if([keyPath isEqualToString:NSStringFromSelector(@selector(currentBlindLevel))]) {
            [self updateButtons];
            [self updateBlinds];
        }

        if([keyPath isEqualToString:NSStringFromSelector(@selector(isConnected))] ||
           [keyPath isEqualToString:NSStringFromSelector(@selector(isAuthorized))]) {
            [self updateButtons];
        }

        if([keyPath isEqualToString:NSStringFromSelector(@selector(seats))]) {
            [self updatePlayers];
            [self updateAverageStack];
        }

        if([keyPath isEqualToString:NSStringFromSelector(@selector(totalChips))]) {
            [self updateAverageStack];
        }

        if([keyPath isEqualToString:NSStringFromSelector(@selector(actionClockTimeRemaining))]) {
            [self updateActionClock];
        }
    }
}

#pragma mark Actions

- (IBAction)previousRoundTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[TournamentSession sharedSession] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[TournamentSession sharedSession] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[TournamentSession sharedSession] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[TournamentSession sharedSession] togglePauseGame];
    } else {
        [[TournamentSession sharedSession] startGameAt:nil];
    }
}

- (IBAction)nextRoundTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[TournamentSession sharedSession] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[TournamentSession sharedSession] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[TournamentSession sharedSession] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        NSUInteger remaining = [[[TournamentSession sharedSession] actionClockTimeRemaining] unsignedIntegerValue];
        if(remaining == 0) {
            [[TournamentSession sharedSession] setActionClock:[NSNumber numberWithUnsignedInteger:kActionClockRequestTime]];
        } else {
            [[TournamentSession sharedSession] setActionClock:nil];
        }
    }
}

# pragma mark TBActionClockViewDelegate

- (CGFloat)analogClock:(TBActionClockView*)clock graduationLengthForIndex:(NSInteger)index {
    return index % 5 == 0 ? 10.0 : 5.0;
}

@end
