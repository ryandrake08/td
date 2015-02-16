//
//  TBFundingTableCellView.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 2/8/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBFundingTableCellView.h"

#define kFundingTypeIndexBuyin 0
#define kFundingTypeIndexRebuy 1
#define kFundingTypeIndexAddon 2

@implementation TBFundingTableCellView

- (void)setObjectValue:(id)objectValue {
    [super setObjectValue:objectValue];

    if([self objectValue]) {
        // Handle type/forbid_after
        NSNumber* isAddon = [[self objectValue] objectForKey:@"is_addon"];
        NSNumber* forbidAfter = [[self objectValue] objectForKey:@"forbid_after_blind_level"];
        BOOL isBuyin = ![isAddon boolValue] && forbidAfter && [forbidAfter intValue] == 0;

        [[self forbidButton] setHidden:isBuyin];
        [[self forbidButton] setState:forbidAfter ? NSOnState : NSOffState];

        [[self untilButton] setHidden:isBuyin || !forbidAfter];
        if(forbidAfter) {
            [[self untilButton] selectItemAtIndex:[forbidAfter integerValue]];
        }

        if([isAddon boolValue]) {
            [[self typeButton] selectItemAtIndex:kFundingTypeIndexAddon];
        } else if(isBuyin) {
            [[self typeButton] selectItemAtIndex:kFundingTypeIndexBuyin];
        } else {
            [[self typeButton] selectItemAtIndex:kFundingTypeIndexRebuy];
        }
    }
}

- (void)setBlindLevels:(NSUInteger)levels {
    // Set up until button
    [[self untilButton] removeAllItems];
    [[self untilButton] addItemWithTitle:NSLocalizedString(@"Tournament Start", nil)];

    for(NSUInteger i=1; i<levels; i++) {
        [[self untilButton] addItemWithTitle:[NSString localizedStringWithFormat:@"Round %ld", i]];
    }
}

#pragma mark Controls

- (IBAction)typeButtonDidChange:(id)sender {
    NSPopUpButton* button = sender;
    NSLog(@"typeButtonDidChange: index == %ld", [button indexOfSelectedItem]);
    switch([button indexOfSelectedItem]) {
        case kFundingTypeIndexBuyin:
            [[self forbidButton] setHidden:YES];
            [[self untilButton] setHidden:YES];
            [[self untilButton] selectItemAtIndex:0];
            [[self objectValue] setObject:[NSNumber numberWithBool:NO] forKey:@"is_addon"];
            [[self objectValue] setObject:[NSNumber numberWithInteger:0] forKey:@"forbid_after_blind_level"];
            break;

        case kFundingTypeIndexRebuy:
            [[self forbidButton] setHidden:NO];
            [[self untilButton] setHidden:[[self forbidButton] state] == NSOffState];
            [[self objectValue] setObject:[NSNumber numberWithBool:NO] forKey:@"is_addon"];
            break;

        case kFundingTypeIndexAddon:
            [[self forbidButton] setHidden:NO];
            [[self untilButton] setHidden:[[self forbidButton] state] == NSOffState];
            [[self objectValue] setObject:[NSNumber numberWithBool:YES] forKey:@"is_addon"];
            break;
    }
}

- (IBAction)forbidButtonDidChange:(id)sender {
    NSButton* button = sender;
    NSLog(@"forbidButtonDidChange: state == %ld", [button state]);
    if([button state] == NSOffState) {
        [[self untilButton] setHidden:YES];
        [[self objectValue] removeObjectForKey:@"forbid_after_blind_level"];
    } else {
        [[self untilButton] setHidden:NO];
        NSInteger untilValue = [[self untilButton] indexOfSelectedItem];
        [[self objectValue] setObject:[NSNumber numberWithInteger:untilValue] forKey:@"forbid_after_blind_level"];
    }
}

- (IBAction)untilButtonDidChange:(id)sender {
    NSPopUpButton* button = sender;
    NSLog(@"untilButtonDidChange: index == %ld", [button indexOfSelectedItem]);
    NSInteger untilValue = [button indexOfSelectedItem];
    [[self objectValue] setObject:[NSNumber numberWithInteger:untilValue] forKey:@"forbid_after_blind_level"];
}

@end
