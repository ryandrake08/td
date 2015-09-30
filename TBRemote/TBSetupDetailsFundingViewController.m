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
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"name"];
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
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"type"];
                break;
            }
            case 2:
            {
                [(TBEditableNumberTableViewCell*)cell setFormatter:[self costFormatter]];
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"cost"];
                break;
            }
            case 3:
            {
                [(TBEditableNumberTableViewCell*)cell setFormatter:[self costFormatter]];
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"commission"];
                break;
            }
            case 4:
            {
                [(TBEditableNumberTableViewCell*)cell setFormatter:[self equityFormatter]];
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"equity"];
                break;
            }
            case 5:
            {
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"chips"];
                break;
            }
            case 6:
            {
                [(TBCheckmarkNumberTableViewCell*)cell setObject:[self object]];
                [(TBCheckmarkNumberTableViewCell*)cell setKeyPath:@"forbid_after_blind_level"];
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
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"forbid_after_blind_level"];
                break;
            }
        }
    }

    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    if(indexPath.section == 0) {
        if(indexPath.row == 6) {
            [tableView reloadData];
            [tableView deselectRowAtIndexPath:indexPath animated:YES];
        }
    }
}

@end
