//
//  TBSetupDetailsChipsViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsChipsViewController.h"
#import "TBEllipseView.h"
#import "TBEditableTableViewCell.h"
#import "TBColor+CSS.h"

@interface TBSetupDetailsChipsViewController ()

@end

@implementation TBSetupDetailsChipsViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark UITableViewDataSource

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if(indexPath.section == 0) {
        switch(indexPath.row) {
            case 0:
            {
                UIColor* detail = [TBColor colorWithName:[self object][@"color"] ];
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

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    if(indexPath.section == 0) {
        // create a cell
        switch(indexPath.row) {
            case 0:
                break;
        }
    }
}

@end
