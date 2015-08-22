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
#import "NSObject+FBKVOController.h"

// TBFundingArrayController implements a new object
@interface TBFundingArrayController : NSArrayController

@end

@implementation TBFundingArrayController

- (id)newObject {
    NSString* name = @"[New Buyin, Rebuy or Addon]";
    NSNumber* type = kFundingTypeAddon;
    NSNumber* chips = @5000;
    NSNumber* cost = @10;
    NSNumber* commission = @0;
    NSNumber* equity = @10;

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:name, @"name", type, @"type", chips, @"chips", cost, @"cost", commission, @"commission", equity, @"equity", nil];
}

@end

// TBFundingTableCellView to handle custom bindings
@interface TBFundingTableCellView : NSTableCellView

@end

@implementation TBFundingTableCellView

- (IBAction)forbidButtonDidChange:(id)sender {
    if([sender state] == NSOnState) {
        [self objectValue][@"forbid_after_blind_level"] = @0;
    } else {
        [[self objectValue] removeObjectForKey:@"forbid_after_blind_level"];
    }
}

@end

@interface TBFundingViewController () <NSTableViewDelegate>

// number formatters
@property (strong) IBOutlet TBCurrencyNumberFormatter* costFormatter;
@property (strong) IBOutlet TBCurrencyNumberFormatter* equityFormatter;

@end

@implementation TBFundingViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* nameSort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES];
    NSSortDescriptor* typeSort = [[NSSortDescriptor alloc] initWithKey:@"type" ascending:YES];

    // set sort descriptors for arrays
    [[self arrayController] setSortDescriptors:@[typeSort, nameSort]];

    // register for KVO
    [[[self costFormatter] KVOController] observe:[self configuration] keyPath:@"cost_currency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[self tableView] reloadData];
    }];

    [[[self equityFormatter] KVOController] observe:[self configuration] keyPath:@"equity_currency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[self tableView] reloadData];
    }];
}

- (NSArray*)blindLevelNames {
    NSMutableArray* blindLevelList = [[NSMutableArray alloc] initWithObjects:NSLocalizedString(@"Tournament Start", nil), nil];

    NSInteger blindLevels = [[self configuration][@"blind_levels"] count];
    for(NSInteger i=1; i<blindLevels; i++) {
        [blindLevelList addObject:[NSString localizedStringWithFormat:@"Round %ld", i]];
    }

    return blindLevelList;
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
