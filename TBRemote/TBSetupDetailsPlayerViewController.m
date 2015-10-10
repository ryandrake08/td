//
//  TBSetupDetailsPlayerViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsPlayerViewController.h"
#import "TBEditableTableViewCell.h"

@implementation TBSetupDetailsPlayerViewController

#pragma mark UITableViewDataSource

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if(indexPath.section == 0) {
        switch(indexPath.row) {
            case 0:
            {
                [(TBEditableTextTableViewCell*)cell setObject:[self object]];
                [(TBEditableTextTableViewCell*)cell setKeyPath:@"name"];
                break;
            }
        }
    }
    return cell;
}

@end
