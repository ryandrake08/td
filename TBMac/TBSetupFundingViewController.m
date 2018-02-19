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
#import "TBPopoverSegue.h"
#import "TBSetupFundingDetailsViewController.h"
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

@implementation TBSetupFundingViewController

- (void)viewDidLoad {
    [super viewDidLoad];

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

- (BOOL)shouldPerformSegueWithIdentifier:(NSStoryboardSegueIdentifier)identifier sender:(id)sender {
    return  [identifier isEqualToString:@"presentFundingDetailsView"] && [[sender superview] isKindOfClass:[NSTableCellView class]];
}

- (void)prepareForSegue:(NSStoryboardSegue*)segue sender:(id)sender {
    // reference the container view controllers
    if([[segue identifier] isEqualToString:@"presentFundingDetailsView"]) {
        TBSetupFundingDetailsViewController* vc = (TBSetupFundingDetailsViewController*)[segue destinationController];

        // set configuration (so TBSetupFundingDetailsViewController knows payout currency)
        [vc setConfiguration:[self representedObject]];

        if([[sender superview] isKindOfClass:[NSTableCellView class]]) {
            NSTableCellView* cellView = (NSTableCellView*)[sender superview];

            // funding source is the table view cell's objectValue
            NSDictionary* fundingSource = [cellView objectValue];
            [vc setRepresentedObject:fundingSource];

            // TODO: this is a hack. we bind the funding details display to the fundingSource dictionary itself,
            // which does not actually change when its key values change. the details popup may change these values
            // and the display must be updated. so observe here:
            [[self KVOController] observe:fundingSource keyPaths:@[@"cost.amount", @"cost.currency", @"commission.amount", @"commission.currency"] options:0 block:^(id  observer, id  object, NSDictionary* change) {
                [cellView setObjectValue:[fundingSource mutableCopy]];
            }];

            // set popup behavior
            if([segue isKindOfClass:[TBPopoverSegue class]]) {
                TBPopoverSegue* popoverSegue = (TBPopoverSegue*)segue;
                [popoverSegue setAnchorView:sender];
                [popoverSegue setPreferredEdge:NSMaxXEdge];
                [popoverSegue setPopoverBehavior:NSPopoverBehaviorTransient];
            }
        }
    }
}

- (NSArray*)blindLevelNames {
    return [TournamentSession blindLevelNamesForConfiguration:[self representedObject]];
}

- (NSArray*)currencyList {
    return [TBCurrencyNumberFormatter supportedCurrencies];
}

#pragma mark Actions

- (IBAction)fundingDetailsButtonDidChange:(id)sender {
    [self performSegueWithIdentifier:@"presentFundingDetailsView" sender:sender];
}

@end
