//
//  TBSetupDetailsRoundsViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsRoundsViewController.h"
#import "TBDurationNumberFormatter.h"

@interface TBSetupDetailsRoundsViewController ()

@property (nonatomic, strong) TBDurationNumberFormatter* durationFormatter;
@end

@implementation TBSetupDetailsRoundsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    [self setDurationFormatter:[[TBDurationNumberFormatter alloc] init]];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

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
                NSString* detail = [[self durationFormatter] stringForObjectValue:[self object][@"duration"]];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 1:
            {
                NSString* detail = [[self object][@"little_blind"] stringValue];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 2:
            {
                NSString* detail = [[self object][@"big_blind"] stringValue];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 3:
            {
                NSString* detail = [[self object][@"ante"] stringValue];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 4:
            {
                if([self object][@"break_duration"] != nil) {
                    [cell setAccessoryType:UITableViewCellAccessoryCheckmark];
                } else {
                    [cell setAccessoryType:UITableViewCellAccessoryNone];
                }
                break;
            }
        }
    } else if(indexPath.section == 1) {
        switch(indexPath.row) {
            case 0:
            {
                NSString* detail = [[self durationFormatter] stringForObjectValue:[self object][@"break_duration"]];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 1:
            {
                NSString* detail = [self object][@"reason"];
                [[cell detailTextLabel] setText:detail];
                break;
            }
        }
    }

    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    if(indexPath.section == 0) {
        // create a cell
        switch(indexPath.row) {
            case 0:
                break;
        }
    }

    // deselect
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
