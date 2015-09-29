//
//  TBSetupDetailsFundingViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsFundingViewController.h"
#import "TBEditableTableViewCell.h"
#import "TBCurrencyNumberFormatter.h"
#import "TournamentSession.h"

@interface TBSetupDetailsFundingViewController ()

// formatters
@property (nonatomic, strong) TBCurrencyNumberFormatter* costFormatter;
@property (nonatomic, strong) TBCurrencyNumberFormatter* equityFormatter;

@end

@implementation TBSetupDetailsFundingViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // create number formatters
    [self setCostFormatter:[[TBCurrencyNumberFormatter alloc] init]];
    [self setEquityFormatter:[[TBCurrencyNumberFormatter alloc] init]];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    if([self object][@"forbid_after_blind_level"] != nil) {
        return 2;
    } else {
        return 1;
    }
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    if(section == 0) {
        return 7;
    } else if(section == 1 && [self object][@"forbid_after_blind_level"] != nil) {
        return 1;
    }
    return 0;
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if(indexPath.section == 0) {
        switch(indexPath.row) {
            case 0:
            {
                [(TBEditableTextTableViewCell*)cell setEditableObject:[self object] keypath:@"name"];
                break;
            }
            case 1:
            {
                [(TBPickableTextTableViewCell*)cell setAllowedValues:@[kFundingTypeBuyin,
                                                                       kFundingTypeRebuy,
                                                                       kFundingTypeAddon]
                                                          withTitles:@[NSLocalizedString(@"Buyin", nil),
                                                                       NSLocalizedString(@"Rebuy", nil),
                                                                       NSLocalizedString(@"Addon", nil)]];
                [(TBPickableTextTableViewCell*)cell setEditableObject:[self object] keypath:@"type"];
                break;
            }
            case 2:
            {
                [(TBEditableNumberTableViewCell*)cell setFormatter:[self costFormatter]];
                [(TBEditableNumberTableViewCell*)cell setEditableObject:[self object] keypath:@"cost"];
                break;
            }
            case 3:
            {
                [(TBEditableNumberTableViewCell*)cell setFormatter:[self costFormatter]];
                [(TBEditableNumberTableViewCell*)cell setEditableObject:[self object] keypath:@"commission"];
                break;
            }
            case 4:
            {
                [(TBEditableNumberTableViewCell*)cell setFormatter:[self equityFormatter]];
                [(TBEditableNumberTableViewCell*)cell setEditableObject:[self object] keypath:@"equity"];
                break;
            }
            case 5:
            {
                [(TBEditableNumberTableViewCell*)cell setEditableObject:[self object] keypath:@"chips"];
                break;
            }
            case 6:
            {
                if([self object][@"forbid_after_blind_level"] != nil) {
                    [cell setAccessoryType:UITableViewCellAccessoryCheckmark];
                } else {
                    [cell setAccessoryType:UITableViewCellAccessoryNone];
                }
                break;
            }
        }
    } else if(indexPath.section == 1) {
        switch(indexPath.row) {
            case 0:
            {
                NSMutableArray* blindLevelIndices = [[NSMutableArray alloc] init];
                for(NSUInteger i=0; i<[[self blindLevels] count]; i++) [blindLevelIndices addObject:@(i)];
                [(TBPickableTextTableViewCell*)cell setAllowedValues:blindLevelIndices withTitles:[TournamentSession namesForBlindLevels:[self blindLevels]]];
                [(TBPickableTextTableViewCell*)cell setEditableObject:[self object] keypath:@"forbid_after_blind_level"];
                break;
            }
        }
    }

    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    if(indexPath.section == 0) {
        // create a cell
        switch(indexPath.row) {
            case 0:
                break;
        }
    }

    // deselect
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
