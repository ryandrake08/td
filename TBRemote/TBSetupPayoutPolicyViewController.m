//
//  TBSetupPayoutsViewController.m
//  TBPhone
//
//  Created by Ryan Drake on 2/4/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupPayoutPolicyViewController.h"
#import "NSObject+FBKVOController.h"
#import "TBSetupTableViewController.h"

@implementation TBSetupPayoutPolicyViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // whenever payout policy changes, checkmarks will change
    [[self KVOController] observe:self keyPath:@"configuration.payout_policy" options:0 block:^(id observer, id object, NSDictionary *change) {
        // reload table
        [[self tableView] reloadData];
    }];
}

#pragma mark Navigation

- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    UIViewController* destinationController = [segue destinationViewController];

    // if the destination a navigation controller, we pass our info to the first object, which should respond to setConfiguration
    if([destinationController isKindOfClass:[UINavigationController class]]) {
        UINavigationController* navController = (UINavigationController*)destinationController;
        destinationController = [[navController viewControllers] firstObject];
    }

    // if we can set a configuration, set it
    if([destinationController respondsToSelector:@selector(setConfiguration:)]) {
        [destinationController performSelector:@selector(setConfiguration:) withObject:[self configuration]];
    }
}

#pragma mark UITableViewDataSource

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if([indexPath row] == [[self configuration][@"payout_policy"] integerValue]) {
        [cell setAccessoryType:UITableViewCellAccessoryCheckmark];
    } else {
        [cell setAccessoryType:UITableViewCellAccessoryNone];
    }

    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    // set payout policy
    [self configuration][@"payout_policy"] = @([indexPath row]);
}

@end
