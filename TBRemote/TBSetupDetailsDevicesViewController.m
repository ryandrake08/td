//
//  TBSetupDetailsDevicesViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsDevicesViewController.h"
#import "TBKVOTableViewCell.h"
#import "TBAuthCodeNumberFormatter.h"

@implementation TBSetupDetailsDevicesViewController

#pragma mark UITableViewDataSource

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if(indexPath.section == 0 && indexPath.row == 0) {
        TBAuthCodeNumberFormatter* codeFormatter = [[TBAuthCodeNumberFormatter alloc] init];
        [(TBEditableTextTableViewCell*)cell setFormatter:codeFormatter];
    }
    
    [(TBKVOTableViewCell*)cell setObject:[self object]];
    return cell;
}

@end
