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

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return [[self arrangedObjects] count]-1;
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
    NSDictionary* object = [self arrangedObjects][[indexPath row]+1];
    TBDurationNumberFormatter* durationFormatter = [[TBDurationNumberFormatter alloc] init];
    [[cell textLabel] setText:[durationFormatter stringForObjectValue:object[@"duration"]]];
    [[cell detailTextLabel] setText:[self formattedRoundStringForSource:object]];
    return cell;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete from the data source
        [[self arrangedObjects] removeObjectAtIndex:[indexPath row]+1];

        // Remove from the table
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

// returns the object for a given indexPath
- (id)arrangedObjectForIndexPath:(NSIndexPath*)indexPath {
    return [self arrangedObjects][[indexPath row]+1];
}

// adds a new object to arrangedObjects
- (void)addArrangedObject:(id)anObject {
    NSIndexPath* newIndexPath = [NSIndexPath indexPathForRow:[[self arrangedObjects] count]-1 inSection:0];

    // Add to the data source
    [[self arrangedObjects] addObject:anObject];

    // Insert to the table
    [[self tableView] insertRowsAtIndexPaths:@[newIndexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
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
