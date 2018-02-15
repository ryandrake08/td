//
//  TBSetupFundingViewController.m
//  td
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSetupFundingViewController.h"
#import "NSObject+FBKVOController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TournamentSession.h"

// TBSetupFundingArrayController implements a new object
@interface TBSetupFundingArrayController : NSArrayController

@property (copy) NSString* defaultCurrency;

@end

@implementation TBSetupFundingArrayController

- (id)newObject {
    NSString* defaultCurrencyCode = [TBCurrencyNumberFormatter defaultCurrencyCode];
    NSString* name = NSLocalizedString(@"Buyin, Rebuy or Addon Name", @"List of the three types of funding sources");
    NSNumber* type = kFundingTypeAddon;
    NSNumber* chips = @5000;
    NSMutableDictionary* cost = [@{@"amount":@10, @"currency":defaultCurrencyCode} mutableCopy];
    NSMutableDictionary* commission =[ @{@"amount":@0, @"currency":defaultCurrencyCode} mutableCopy];
    NSMutableDictionary* equity = [@{@"amount":@10, @"currency":[self defaultCurrency]} mutableCopy];

    return [@{@"name":name, @"type":type, @"chips":chips, @"cost":cost, @"commission":commission, @"equity":equity} mutableCopy];
}

@end

// TBSetupFundingTableCellView to handle custom bindings
@interface TBSetupFundingTableCellView : NSTableCellView

@end

@implementation TBSetupFundingTableCellView

- (IBAction)forbidButtonDidChange:(NSButton*)sender {
    if([sender state] == NSOnState) {
        [self objectValue][@"forbid_after_blind_level"] = @0;
    } else {
        [[self objectValue] removeObjectForKey:@"forbid_after_blind_level"];
    }
}

@end

@interface TBSetupFundingViewController ()

@property (strong) NSArray* currencyList;

@end

@implementation TBSetupFundingViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // set up currency list
    _currencyList = [TBCurrencyNumberFormatter supportedCurrencies];
    [self willChangeValueForKey:@"currencyList"];
    [self didChangeValueForKey:@"currencyList"];

    // setup sort descriptors
    NSSortDescriptor* nameSort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES];
    NSSortDescriptor* typeSort = [[NSSortDescriptor alloc] initWithKey:@"type" ascending:YES];

    // set sort descriptors for arrays
    [[self arrayController] setSortDescriptors:@[typeSort, nameSort]];

    // observe change to payout currency
    [[self KVOController] observe:self keyPath:@"representedObject.payout_currency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        id payoutCurrency = [self representedObject][@"payout_currency"];
        // set default currency for new objects
        [(TBSetupFundingArrayController*)[self arrayController] setDefaultCurrency:payoutCurrency];
        [[self tableView] reloadData];
    }];
}

- (NSArray*)blindLevelNames {
    NSMutableArray* names = [[NSMutableArray alloc] initWithObjects:NSLocalizedString(@"Tournament Start", nil), nil];
    for(NSInteger i=1; i<[[self representedObject][@"blind_levels"] count]; i++) {
        [names addObject:[NSString stringWithFormat:NSLocalizedString(@"Round %ld", @"Numbered blind level"), i]];
    }
    return names;
}

@end
