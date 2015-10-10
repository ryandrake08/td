//
//  TBSetupDetailsChipsViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsChipsViewController.h"
#import "TBChooseColorViewController.h"
#import "TBEllipseView.h"
#import "TBEditableTableViewCell.h"
#import "TBColor+CSS.h"

@implementation TBSetupDetailsChipsViewController

#pragma mark UITableViewDataSource

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if(indexPath.section == 0) {
        switch(indexPath.row) {
            case 0:
            {
                UIColor* detail = [TBColor colorWithName:[self object][@"color"]];
                [(TBEllipseView*)[cell viewWithTag:100] setColor:detail];
                break;
            }
            case 1:
            {
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"denomination"];
                break;
            }
            case 2:
            {
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"count_available"];
                break;
            }
        }
    }
    return cell;
}

#pragma mark Navigation

- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    [super prepareForSegue:segue sender:sender];

    TBChooseColorViewController* newController = [segue destinationViewController];
    [newController setObject:[self object]];
}

@end
