//
//  TBSetupFundingViewController.m
//  td
//
//  Created by Ryan Drake on 9/23/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupFundingViewController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TBFundingSourceFormatter.h"
#import "TBSetupDetailsFundingViewController.h"
#import "TournamentSession.h"

@implementation TBSetupFundingViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setArrangedObjectsKeyPath:@"configuration.funding_sources"];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (UIImage*)imageForSource:(NSDictionary*)fundingSource {
    if([fundingSource[@"type"] isEqual:kFundingTypeBuyin]) {
        return [UIImage imageNamed:@"m_buyin"];
    } else if([fundingSource[@"type"] isEqual:kFundingTypeRebuy]) {
        return [UIImage imageNamed:@"m_rebuy"];
    } else if([fundingSource[@"type"] isEqual:kFundingTypeAddon]) {
        return [UIImage imageNamed:@"m_addon"];
    }
    return nil;
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SetupFundingCell" forIndexPath:indexPath];
    NSDictionary* object = [super arrangedObjectForIndexPath:indexPath];
    [(UIImageView*)[cell viewWithTag:100] setImage:[self imageForSource:object]];
    [(UILabel*)[cell viewWithTag:101] setText:object[@"name"]];
    TBFundingSourceFormatter* fundingFormatter = [[TBFundingSourceFormatter alloc] init];
    [(UILabel*)[cell viewWithTag:102] setText:[fundingFormatter stringForObjectValue:object]];

    return cell;
}

- (id)newObject {
    NSString* defaultCurrencyCode = [TBCurrencyNumberFormatter defaultCurrencyCode];
    NSString* name = NSLocalizedString(@"Buyin, Rebuy or Addon Name", nil);
    NSNumber* type = kFundingTypeAddon;
    NSNumber* chips = @5000;
    NSMutableDictionary* cost = [@{@"amount":@10, @"currency":defaultCurrencyCode} mutableCopy];
    NSMutableDictionary* commission =[ @{@"amount":@0, @"currency":defaultCurrencyCode} mutableCopy];
    NSMutableDictionary* equity = [@{@"amount":@10} mutableCopy];

    return [@{@"name":name, @"type":type, @"chips":chips, @"cost":cost, @"commission":commission, @"equity":equity} mutableCopy];
}

@end
