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

    if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"code"]) {
        TBAuthCodeNumberFormatter* codeFormatter = [[TBAuthCodeNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:codeFormatter];
    }

    return [self setObjectToCell:cell];
}

@end
