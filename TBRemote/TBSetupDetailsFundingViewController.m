//
//  TBSetupDetailsFundingViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsFundingViewController.h"
#import "TournamentSession.h"

@interface TBSetupDetailsFundingViewController ()

@property (nonatomic, strong) NSArray* fundingTypes;

@end

@implementation TBSetupDetailsFundingViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    [self setFundingTypes:@[NSLocalizedString(@"Buyin", nil),
                            NSLocalizedString(@"Rebuy", nil),
                            NSLocalizedString(@"Addon", nil)]];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    if([self object][@"forbid_after_blind_level"] != nil) {
        return 2;
    } else {
        return 1;
    }
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    if(section == 0) {
        return 7;
    } else if(section == 1 && [self object][@"forbid_after_blind_level"] != nil) {
        return 1;
    }
    return 0;
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if(indexPath.section == 0) {
        switch(indexPath.row) {
            case 0:
            {
                NSString* detail = [self object][@"name"];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 1:
            {
                NSInteger typeIndex = [[self object][@"type"] integerValue];
                NSString* detail = [self fundingTypes][typeIndex];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 2:
            {
                NSString* detail = [[self object][@"cost"] stringValue];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 3:
            {
                NSString* detail = [[self object][@"commission"] stringValue];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 4:
            {
                NSString* detail = [[self object][@"equity"] stringValue];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 5:
            {
                NSString* detail = [[self object][@"chips"] stringValue];
                [[cell detailTextLabel] setText:detail];
                break;
            }
            case 6:
            {
                if([self object][@"forbid_after_blind_level"] != nil) {
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
                NSString* detail = [[self object][@"forbid_after_blind_level"] stringValue];
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
