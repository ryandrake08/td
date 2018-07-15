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
    [self setArrangedObjectsKeyPath:@"configuration.blind_levels"];

    // ensure we have at lease on round. first round is a "dummy" round and must exist!
    if([[self arrangedObjects] count] == 0 ) {
        NSLog(@"Warning: invalid configuration! must have the initial (setup) round");
        // create a dummy round here
        [[self arrangedObjects] addObject:@{}];
    }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (NSString*)formattedStringForRound:(NSDictionary*)round {
    NSString* formattedRound;
    if([round[@"ante"] doubleValue] != 0.0) {
        NSString* anteLabel = [round[@"big_blind_ante"] boolValue] ? NSLocalizedString(@"BBA", @"Big Blind Ante") : NSLocalizedString(@"Ante", nil);
        formattedRound = [NSString stringWithFormat:@"%@/%@ %@:%@", round[@"little_blind"], round[@"big_blind"], anteLabel, round[@"ante"]];
    } else {
        formattedRound = [NSString stringWithFormat:@"%@/%@", round[@"little_blind"], round[@"big_blind"]];
    }
    return formattedRound;
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SetupRoundsCell" forIndexPath:indexPath];
    NSDictionary* object = [self arrangedObjects][[indexPath row]];
    TBDurationNumberFormatter* durationFormatter = [[TBDurationNumberFormatter alloc] init];
    [[cell textLabel] setText:[durationFormatter stringForObjectValue:object[@"duration"]]];
    [[cell detailTextLabel] setText:[self formattedStringForRound:object]];
    return cell;
}

- (CGFloat)tableView:(UITableView*)tableView heightForRowAtIndexPath:(NSIndexPath*)indexPath {
    if([indexPath row] == 0) {
        // Hide first (dummy round) row
        return 0;
    } else {
        return 43.5;
    }
}

- (id)newObject {
    NSString* game_name = NSLocalizedString(@"No Limit Texas Hold'em", nil);
    NSNumber* little_blind = @25;
    NSNumber* big_blind = @50;
    NSNumber* ante = @0;
    NSNumber* big_blind_ante = @NO;
    NSNumber* duration = @3600000;

    if([[self arrangedObjects] count] > 1) {
        // base new round on final round x 2
        NSDictionary* last = [[self arrangedObjects] lastObject];
        game_name = last[@"game_name"];
        little_blind = @([last[@"little_blind"] intValue] * 2);
        big_blind = @([last[@"big_blind"] intValue] * 2);
        ante = @([last[@"ante"] intValue] * 2);
        big_blind_ante = last[@"big_blind_ante"];
        duration = last[@"duration"];
    }

    return [@{@"game_name":game_name, @"little_blind":little_blind, @"big_blind":big_blind, @"ante":ante, @"big_blind_ante":big_blind_ante, @"duration":duration} mutableCopy];
}

@end
