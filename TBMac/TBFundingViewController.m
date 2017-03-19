//
//  TBFundingViewController.m
//  td
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBFundingViewController.h"
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

- (IBAction)forbidButtonDidChange:(NSButton*)sender {
    if([sender state] == NSOnState) {
        [self objectValue][@"forbid_after_blind_level"] = @0;
    } else {
        [[self objectValue] removeObjectForKey:@"forbid_after_blind_level"];
    }
}

@end

@interface TBFundingViewController ()

@end

@implementation TBFundingViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* nameSort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES];
    NSSortDescriptor* typeSort = [[NSSortDescriptor alloc] initWithKey:@"type" ascending:YES];

    // set sort descriptors for arrays
    [[self arrayController] setSortDescriptors:@[typeSort, nameSort]];
}

- (NSArray*)blindLevelNames {
    return [TournamentSession namesForBlindLevels:[self configuration][@"blind_levels"]];
}

@end
