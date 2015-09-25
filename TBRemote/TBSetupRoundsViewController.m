//
//  TBSetupRoundsViewController.m
//  td
//
//  Created by Ryan Drake on 9/23/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupRoundsViewController.h"
#import "TBDurationNumberFormatter.h"

@implementation TBSetupRoundsViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setArrangedObjects:[self configuration][@"blind_levels"]];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (NSString*)formattedRoundStringForSource:(NSDictionary*)fundingSource {
    NSString* formattedFunding;
    if([fundingSource[@"ante"] doubleValue] != 0.0) {
        formattedFunding = [NSString stringWithFormat:@"%@/%@ A:%@", fundingSource[@"little_blind"], fundingSource[@"big_blind"], fundingSource[@"ante"]];
    } else {
        formattedFunding = [NSString stringWithFormat:@"%@/%@", fundingSource[@"little_blind"], fundingSource[@"big_blind"]];
    }
    return formattedFunding;
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SetupRoundsCell" forIndexPath:indexPath];
    NSDictionary* object = [self arrangedObjects][[indexPath row]];
    TBDurationNumberFormatter* durationFormatter = [[TBDurationNumberFormatter alloc] init];
    [[cell textLabel] setText:[durationFormatter stringForObjectValue:object[@"duration"]]];
    [[cell detailTextLabel] setText:[self formattedRoundStringForSource:object]];
    return cell;
}

- (CGFloat)tableView:(UITableView*)tableView heightForRowAtIndexPath:(NSIndexPath*)indexPath {
    if([indexPath row] == 0) {
        return 0;
    } else {
        return 43.5;
    }
}

- (id)newObject {
    NSString* game_name = @"No Limit Texas Hold'em";
    NSNumber* little_blind = @25;
    NSNumber* big_blind = @50;
    NSNumber* ante = @0;
    NSNumber* duration = @3600000;

    NSDictionary* last = [[self arrangedObjects] lastObject];
    if(last != nil) {
        game_name = last[@"game_name"];
        little_blind = @([last[@"little_blind"] intValue] * 2);
        big_blind = @([last[@"big_blind"] intValue] * 2);
        ante = @([last[@"ante"] intValue] * 2);
        duration = last[@"duration"];
    }

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:game_name, @"game_name", little_blind, @"little_blind", big_blind, @"big_blind", ante, @"ante", duration, @"duration", nil];
}

- (IBAction)addItem:(id)sender {
    [self addArrangedObject:[self newObject]];
}

@end
