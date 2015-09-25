//
//  TBSetupDetailsDevicesViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsDevicesViewController.h"
#import "TBAuthCodeNumberFormatter.h"

@interface TBSetupDetailsDevicesViewController ()

@property (nonatomic, strong) TBAuthCodeNumberFormatter* codeFormatter;

@end

@implementation TBSetupDetailsDevicesViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // create number formatter
    [self setCodeFormatter:[[TBAuthCodeNumberFormatter alloc] init]];
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
                NSString* detail = [[self codeFormatter] stringFromNumber:[self object][@"code"]];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 1:
            {
                NSString* detail = [self object][@"name"];
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
