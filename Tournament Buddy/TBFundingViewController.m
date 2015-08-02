//
//  TBFundingViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBFundingViewController.h"
#import "NSObject+FBKVOController.h"
#import "TBCurrencyNumberFormatter.h"

// TBFundingTableCellView to handle custom bindings
@interface TBFundingTableCellView : NSTableCellView

@end

@implementation TBFundingTableCellView

#define kFundingTypeIndexBuyin 0
#define kFundingTypeIndexRebuy 1
#define kFundingTypeIndexAddon 2

- (void)setObjectValue:(id)objectValue {
    [self willChangeValueForKey:@"typeButtonSelectedTag"];
    [self willChangeValueForKey:@"isBuyin"];
    [super setObjectValue:objectValue];
    [self didChangeValueForKey:@"typeButtonSelectedTag"];
    [self didChangeValueForKey:@"isBuyin"];
}

- (IBAction)forbidButtonDidChange:(id)sender {
    if([sender state] == NSOnState) {
        [[self objectValue] setObject:@0 forKey:@"forbid_after_blind_level"];
    } else {
        [[self objectValue] removeObjectForKey:@"forbid_after_blind_level"];
    }
}

- (BOOL)isBuyin {
    return [self typeButtonSelectedTag] == kFundingTypeIndexBuyin;
}

- (NSInteger)typeButtonSelectedTag {
    NSNumber* isAddon = [self objectValue][@"is_addon"];
    NSNumber* forbidAfterBlindLevel = [self objectValue][@"forbid_after_blind_level"];
    if([isAddon boolValue]) {
        return kFundingTypeIndexAddon;
    } else if(forbidAfterBlindLevel && [forbidAfterBlindLevel integerValue] == 0) {
        return kFundingTypeIndexBuyin;
    } else {
        return kFundingTypeIndexRebuy;
    }
}

- (void)setTypeButtonSelectedTag:(NSInteger)typeButtonSelectedTag {
    switch(typeButtonSelectedTag) {
        case kFundingTypeIndexBuyin:
            [[self objectValue] setObject:@NO forKey:@"is_addon"];
            [[self objectValue] setObject:@0 forKey:@"forbid_after_blind_level"];
            break;

        case kFundingTypeIndexRebuy:
            [[self objectValue] setObject:@NO forKey:@"is_addon"];
            if([self objectValue][@"forbid_after_blind_level"] && [[self objectValue][@"forbid_after_blind_level"] integerValue] == 0) {
                [[self objectValue] removeObjectForKey:@"forbid_after_blind_level"];
            } else {
                [[self objectValue] willChangeValueForKey:@"forbid_after_blind_level"];
                [[self objectValue] didChangeValueForKey:@"forbid_after_blind_level"];
            }
            break;

        case kFundingTypeIndexAddon:
            [[self objectValue] setObject:@YES forKey:@"is_addon"];
            [[self objectValue] willChangeValueForKey:@"forbid_after_blind_level"];
            [[self objectValue] didChangeValueForKey:@"forbid_after_blind_level"];
            break;
    }
}

@end

@interface TBFundingViewController () <NSTableViewDelegate>

@property (strong) IBOutlet TBCurrencyNumberFormatter* costFormatter;
@property (strong) IBOutlet TBCurrencyNumberFormatter* equityFormatter;

@end

@implementation TBFundingViewController

- (void)viewDidLoad {
    // register for KVO
    [[[self costFormatter] KVOController] observe:[self configuration] keyPath:@"cost_currency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [observer setCurrencyCode:object[@"cost_currency"]];
        [[self tableView] reloadData];
    }];

    [[[self equityFormatter] KVOController] observe:[self configuration] keyPath:@"equity_currency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [observer setCurrencyCode:object[@"equity_currency"]];
        [[self tableView] reloadData];
    }];
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView*)aTableView viewForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    NSTableCellView* result = [aTableView makeViewWithIdentifier:aTableColumn.identifier owner:self];
    if([[aTableColumn identifier] isEqualToString:@"Cost"] || [[aTableColumn identifier] isEqualToString:@"Fee"]) {
        [[result textField] setFormatter:[self costFormatter]];
    } else if([[aTableColumn identifier] isEqualToString:@"Equity"]) {
        [[result textField] setFormatter:[self equityFormatter]];
    }
    return result;
}

@end
