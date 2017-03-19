//
//  TBSetupFundingViewController.m
//  td
//
//  Created by Ryan Drake on 9/23/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupFundingViewController.h"
#import "TBSetupDetailsFundingViewController.h"
#import "TournamentSession.h"

@implementation TBSetupFundingViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setArrangedObjects:[self configuration][@"funding_sources"]];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (NSString*)formattedFundingStringForSource:(NSDictionary*)fundingSource {
    NSString* formattedFunding;
    if([fundingSource[@"commission"] doubleValue] != 0.0) {
        if([fundingSource[@"commission_currency"] isEqualToString:fundingSource[@"cost_currency"]]) {
            formattedFunding = [NSString stringWithFormat:@"%@%@+%@", fundingSource[@"cost_currency"], fundingSource[@"cost"], fundingSource[@"commission"]];
        } else {
            formattedFunding = [NSString stringWithFormat:@"%@%@+%@%@", fundingSource[@"cost_currency"], fundingSource[@"cost"], fundingSource[@"commission_currency"], fundingSource[@"commission"]];
        }
    } else {
        formattedFunding = [NSString stringWithFormat:@"%@%@", fundingSource[@"cost_currency"], fundingSource[@"cost"]];
    }
    return formattedFunding;
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
    [(UILabel*)[cell viewWithTag:102] setText:[self formattedFundingStringForSource:object]];

    return cell;
}

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
