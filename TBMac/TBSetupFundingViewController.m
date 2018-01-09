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

@end

@implementation TBSetupFundingArrayController

- (id)newObject {
    NSString* name = NSLocalizedString(@"Buyin, Rebuy or Addon Name", nil);
    NSNumber* type = kFundingTypeAddon;
    NSNumber* chips = @5000;
    NSNumber* cost = @10;
    NSNumber* commission = @0;
    NSNumber* equity = @10;

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

@implementation TBSetupFundingViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* nameSort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES];
    NSSortDescriptor* typeSort = [[NSSortDescriptor alloc] initWithKey:@"type" ascending:YES];

    // set sort descriptors for arrays
    [[self arrayController] setSortDescriptors:@[typeSort, nameSort]];
}

- (NSArray*)blindLevelNames {
    return [TournamentSession namesForBlindLevels:[self representedObject][@"blind_levels"]];
}

@end
