//
//  TBSetupDetailsDevicesViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsDevicesViewController.h"
#import "TBEditableTableViewCell.h"
#import "TBAuthCodeNumberFormatter.h"

@implementation TBSetupDetailsDevicesViewController

#pragma mark UITableViewDataSource

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if(indexPath.section == 0) {
        switch(indexPath.row) {
            case 0:
            {
                TBAuthCodeNumberFormatter* codeFormatter = [[TBAuthCodeNumberFormatter alloc] init];
                [(TBEditableNumberTableViewCell*)cell setFormatter:codeFormatter];
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"code"];
                break;
            }
            case 1:
            {
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                [(TBEditableTableViewCell*)cell setKeyPath:@"name"];
                break;
            }
        }
    }
    return cell;
}

@end
