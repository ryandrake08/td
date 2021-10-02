//
//  TBSetupRoundsViewController.m
//  td
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSetupRoundsViewController.h"
#import "TBSetupRoundsDetailsViewController.h"
#import "TBMacDocument.h"
#import "TournamentSession.h"

// TBSetupRoundsArrayController implements a new object
@interface TBSetupRoundsArrayController : NSArrayController

@end

@implementation TBSetupRoundsArrayController

- (id)newObject {
    NSNumber* little_blind = @25;
    NSNumber* big_blind = @50;
    NSNumber* ante = @0;
    NSNumber* ante_type = kAnteTypeNone;
    NSNumber* duration = @3600000;

    NSDictionary* last = [[self arrangedObjects] lastObject];
    if(last != nil) {
        if(last[@"little_blind"] != nil) {
            little_blind = @([last[@"little_blind"] intValue] * 2);
        }
        if(last[@"big_blind"] != nil) {
            big_blind = @([last[@"big_blind"] intValue] * 2);
        }
        if(last[@"ante"] != nil) {
            ante = @([last[@"ante"] intValue] * 2);
        }
        if(last[@"ante_type"] != nil) {
            ante_type = last[@"ante_type"];
        }
        if(last[@"duration"] != nil) {
            duration = last[@"duration"];
        }
    }

    return [@{@"little_blind":little_blind, @"big_blind":big_blind, @"ante":ante, @"ante_type":ante_type, @"duration":duration} mutableCopy];
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
    NSPredicate* predicate = [NSPredicate predicateWithFormat: @"%K != %@", @"duration", nil];
    [[self arrayController] setFilterPredicate:predicate];
}

- (void)prepareForSegue:(NSStoryboardSegue*)segue sender:(id)sender {
    // reference the container view controllers
    if([[segue identifier] isEqualToString:@"presentRoundsDetailsView"]) {
        TBSetupRoundsDetailsViewController* vc = (TBSetupRoundsDetailsViewController*)[segue destinationController];

        // set configuration (so TBSetupFundingDetailsViewController knows payout currency)
        [vc setConfiguration:[self representedObject]];

        // get document from sheet parent
        TBMacDocument* document = [[[[[self view] window] sheetParent] windowController] document];

        // set session
        [vc setSession:[document session]];
    }
}

#pragma mark NSTableViewDataSource

- (id)tableView:(NSTableView*)aTableView objectValueForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    if([[aTableColumn identifier] isEqualToString:@"Round"]) {
        return @(rowIndex+1);
    } else if([[aTableColumn identifier] isEqualToString:@"Start Time"]) {
        long totalDuration = 0;
        for(NSUInteger i=0; i<(NSUInteger)rowIndex; i++) {
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
