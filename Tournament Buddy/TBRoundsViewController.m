//
//  TBRoundsViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRoundsViewController.h"

// TBRoundsArrayController implements a new object
@interface TBRoundsArrayController : NSArrayController

@end

@implementation TBRoundsArrayController

- (id)newObject {
    NSString* game_name = @"No Limit Texas Hold'em";
    NSNumber* little_blind = @25;
    NSNumber* big_blind = @50;
    NSNumber* ante = @0;
    NSNumber* duration = @3600000;

    NSDictionary* last = [[self content] lastObject];
    if(last != nil) {
        game_name = last[@"game_name"];
        little_blind = @([last[@"little_blind"] intValue] * 2);
        big_blind = @([last[@"big_blind"] intValue] * 2);
        ante = @([last[@"ante"] intValue] * 2);
        duration = last[@"duration"];
    }

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:game_name, @"game_name", little_blind, @"little_blind", big_blind, @"big_blind", ante, @"ante", duration, @"duration", nil];
}

@end

// TBRoundsTableCellView to simply handle the break button check box
@interface TBRoundsTableCellView : NSTableCellView

@end

@implementation TBRoundsTableCellView

- (IBAction)breakButtonDidChange:(id)sender {
    if([sender state] == NSOnState) {
        [self objectValue][@"break_duration"] = @0;
    } else {
        [[self objectValue] removeObjectForKey:@"break_duration"];
        [[self objectValue] removeObjectForKey:@"reason"];
    }
}
@end

@implementation TBRoundsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // filter predicate to not show fake "setup" round
    NSPredicate* predicate = [NSPredicate predicateWithFormat: @"%K != %@", @"game_name", @"Setup"];
    [[self arrayController] setFilterPredicate:predicate];
}

#pragma mark NSTableViewDataSource

- (id)tableView:(NSTableView*)aTableView objectValueForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    if([[aTableColumn identifier] isEqualToString:@"Round"]) {
        return @(rowIndex+1);
    } else if([[aTableColumn identifier] isEqualToString:@"Start Time"]) {
        long totalDuration = 0;
        for(NSInteger i=0; i<rowIndex; i++) {
            NSDictionary* object = [[self arrayController] arrangedObjects][i];
            long duration = [object[@"duration"] longValue];
            long breakDuration = [object[@"break_duration"] longValue];
            totalDuration += duration + breakDuration;
        }
        return @(totalDuration);
    }
    return nil;
}

@end
