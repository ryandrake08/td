//
//  TBFundingViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBFundingViewController.h"

@implementation TBFundingTableCellView
@end

@interface TBFundingViewController () <NSTableViewDataSource, NSTableViewDelegate>

@end

@implementation TBFundingViewController

// initializer
- (instancetype)initWithSession:(TournamentSession*)sess {
    self = [super initWithNibName:@"TBFundingView" session:sess];
    if(self) {
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

#pragma mark NSTableView

- (NSInteger)numberOfRowsInTableView:(NSTableView*)tableView {

    NSArray* fundingSources = [[self session] fundingSources];
    NSLog(@"Funding Sources: %lu", (unsigned long)[fundingSources count]);
    return (NSInteger)[fundingSources count];
}

- (NSView *)tableView:(NSTableView*)tableView viewForTableColumn:(NSTableColumn*)tableColumn row:(NSInteger)row {

    id source = [[[self session] fundingSources] objectAtIndex:row];

    // Retrieve to get the @"MyView" from the pool or,
    // if no version is available in the pool, load the Interface Builder version
    TBFundingTableCellView* result = [tableView makeViewWithIdentifier:[tableColumn identifier] owner:self];

    // Set up the cell's members
    [[result textField] setStringValue:[source objectForKey:@"name"]];
    [[result chipsField] setObjectValue:[source objectForKey:@"chips"]];
    [[result costField] setObjectValue:[source objectForKey:@"cost"]];
    [[result feeField] setObjectValue:[source objectForKey:@"commission"]];
    [[result equityField] setObjectValue:[source objectForKey:@"equity"]];

    // Set up until button
    [[result untilButton] removeAllItems];
    [[result untilButton] addItemWithTitle:NSLocalizedString(@"Tournament Start", nil)];
    for(int i=1; i<[[[self session] blindLevels] count]; i++) {
        [[result untilButton] addItemWithTitle:[NSString localizedStringWithFormat:@"Round %d", i]];
    }

    // Handle type/forbid_after
    NSNumber* isAddon = [source objectForKey:@"is_addon"];
    NSNumber* forbidAfter = [source objectForKey:@"forbid_after_blind_level"];

    if(![isAddon boolValue] && forbidAfter && [forbidAfter intValue] == 0) {
        [[result typeButton] selectItemAtIndex:0];
        [[result allowedButton] setHidden:YES];
        [[result untilButton] setHidden:YES];
    } else {
        [[result allowedButton] setHidden:NO];

        if(forbidAfter == nil) {
            [[result allowedButton] setState:NSOffState];
            [[result untilButton] setHidden:YES];
        } else {
            [[result allowedButton] setState:NSOnState];
            [[result untilButton] setHidden:NO];
        }

        if([isAddon boolValue]) {
            [[result typeButton] selectItemAtIndex:2];
        } else {
            [[result typeButton] selectItemAtIndex:1];
        }
    }

    // Return the result
    return result;
}

@end
