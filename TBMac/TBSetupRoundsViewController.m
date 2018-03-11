//
//  TBSetupRoundsViewController.m
//  td
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSetupRoundsViewController.h"

// TBSetupRoundsArrayController implements a new object
@interface TBSetupRoundsArrayController : NSArrayController

@end

@implementation TBSetupRoundsArrayController

- (id)newObject {
    NSString* game_name = NSLocalizedString(@"No Limit Texas Hold'em", @"Default name for a poker game");
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

    return [@{@"game_name":game_name, @"little_blind":little_blind, @"big_blind":big_blind, @"ante":ante, @"duration":duration} mutableCopy];
}

@end

// TBSetupRoundsTableCellView to simply handle the break button check box
@interface TBSetupRoundsTableCellView : NSTableCellView

@end

@implementation TBSetupRoundsTableCellView

- (IBAction)breakButtonDidChange:(NSButton*)sender {
    if([sender state] == NSOnState) {
        [self objectValue][@"break_duration"] = @0;
    } else {
        [[self objectValue] removeObjectForKey:@"break_duration"];
        [[self objectValue] removeObjectForKey:@"reason"];
    }
}
@end

@interface TBSetupRoundsViewController () <NSTableViewDataSource>

@end

@implementation TBSetupRoundsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // filter predicate to not show fake "setup" round
    NSPredicate* predicate = [NSPredicate predicateWithFormat: @"%K != %@", @"game_name", nil];
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
