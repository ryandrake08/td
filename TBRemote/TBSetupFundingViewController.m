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
    NSString* costCurrency = [self configuration][@"cost_currency"];
    if([fundingSource[@"commission"] doubleValue] != 0.0) {
        formattedFunding = [NSString stringWithFormat:@"%@+%@ %@", fundingSource[@"cost"], fundingSource[@"commission"], costCurrency];
    } else {
        formattedFunding = [NSString stringWithFormat:@"%@ %@", fundingSource[@"cost"], costCurrency];
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

#pragma mark Navigation

// custom prepareForSegue that passes blind level list
- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    TBSetupDetailsFundingViewController* newController = [segue destinationViewController];
    NSIndexPath* indexPath = [[self tableView] indexPathForSelectedRow];
    [newController setObject:[self arrangedObjectForIndexPath:indexPath]];
    [newController setBlindLevels:[self configuration][@"blind_levels"]];
}

@end
