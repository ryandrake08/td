//
//  TBSetupDetailsRoundsViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsRoundsViewController.h"
#import "TBEditableTableViewCell.h"
#import "TBDurationNumberFormatter.h"

@implementation TBSetupDetailsRoundsViewController

#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    if([self object][@"break_duration"] != nil) {
        return 2;
    } else {
        return 1;
    }
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    if(section == 0) {
        return 5;
    } else if(section == 1 && [self object][@"break_duration"] != nil) {
        return 2;
    }
    return 0;
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if(indexPath.section == 0) {
        switch(indexPath.row) {
            case 0:
            {
                TBDurationNumberFormatter* durationFormatter = [[TBDurationNumberFormatter alloc] init];
                [(TBEditableNumberTableViewCell*)cell setFormatter:durationFormatter];
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"duration"];
                break;
            }
            case 1:
            {
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"little_blind"];
                break;
            }
            case 2:
            {
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"big_blind"];
                break;
            }
            case 3:
            {
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"ante"];
                break;
            }
            case 4:
            {
                [(TBCheckmarkNumberTableViewCell*)cell setObject:[self object]];
                [(TBCheckmarkNumberTableViewCell*)cell setKeyPath:@"break_duration"];
                break;
            }
        }
    } else if(indexPath.section == 1) {
        switch(indexPath.row) {
            case 0:
            {
                TBDurationNumberFormatter* durationFormatter = [[TBDurationNumberFormatter alloc] init];
                [(TBEditableNumberTableViewCell*)cell setFormatter:durationFormatter];
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"break_duration"];
                break;
            }
            case 1:
            {
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"reason"];
                break;
            }
        }
    }

    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    if(indexPath.section == 0) {
       if(indexPath.row == 4) {
           [tableView deselectRowAtIndexPath:indexPath animated:YES];
           [tableView reloadData];
       }
    }
}

@end
