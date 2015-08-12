//
//  TBRoundsViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRoundsViewController.h"
#import "NSObject+FBKVOController.h"

// TBRoundsTableCellView to simply handle the break button check box
@interface TBRoundsTableCellView : NSTableCellView

@end

@implementation TBRoundsTableCellView

- (IBAction)breakButtonDidChange:(id)sender {
    if([sender state] == NSOnState) {
        [[self objectValue] setObject:@0 forKey:@"break_duration"];
    } else {
        [[self objectValue] removeObjectForKey:@"break_duration"];
        [[self objectValue] removeObjectForKey:@"reason"];
    }
}
@end

@implementation TBRoundsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // set up filter predicate
    NSPredicate* predicate = [NSPredicate predicateWithFormat: @"%K != %@", @"game_name", @"Setup"];
    [[self arrayController] setFilterPredicate:predicate];

    // register for KVO on arrangedObjects
    NSArray* keyPaths = @[@"arrangedObjects"];
    [[self KVOController] observe:[self arrayController] keyPaths:keyPaths options:0 block:^(id observer, id object, NSDictionary *change) {
        [[self session] selectiveConfigureAndUpdate:[self configuration]];
    }];
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
