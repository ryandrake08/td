//
//  TBSetupDependsOnTurnoutViewController.m
//  TBPhone
//
//  Created by Ryan Drake on 2/4/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupDependsOnTurnoutViewController.h"

@implementation TBSetupDependsOnTurnoutViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setArrangedObjectsKeyPath:@"configuration.manual_payouts"];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
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
    } else {
        NSLog(@"Warning: Segue destination does not respond to setConfiguration:");
    }

    // set arranged objects to whichever one is selected
    if([destinationController respondsToSelector:@selector(setArrangedObjects:)]) {
        NSIndexPath* selectedIndexPath = [[self tableView] indexPathForSelectedRow];
        NSMutableDictionary* selectedPayout = [self arrangedObjectForIndexPath:selectedIndexPath];
        [destinationController performSelector:@selector(setArrangedObjects:) withObject:selectedPayout[@"payouts"]];
    } else {
        NSLog(@"Warning: Segue destination does not respond to setArrangedObjects:");
    }
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"DependsOnTurnoutCell" forIndexPath:indexPath];
    NSDictionary* object = [super arrangedObjectForIndexPath:indexPath];

    // players
    NSString* playersString = [NSString localizedStringWithFormat:@"%@ players", object[@"buyins_count"]];
    [[cell textLabel] setText:playersString];

    // places paid
    NSString* payoutsCount = [NSString localizedStringWithFormat: @"%ld places paid", (long)[object[@"payouts"] count]];
    [[cell detailTextLabel] setText:payoutsCount];
    return cell;
}

- (id)newObject {
    // new buyin is last one +1
    NSNumber* buyins_count = @1;
    NSDictionary* lastPayout = [[self configuration][@"manual_payouts"] lastObject];
    if(lastPayout != nil) {
        NSNumber* lastBuyinsCount = lastPayout[@"buyins_count"];
        buyins_count = @([lastBuyinsCount integerValue] + 1);
    }
    NSMutableArray* payouts = [[NSMutableArray alloc] init];

    return [@{@"buyins_count":buyins_count, @"payouts":payouts} mutableCopy];
}

@end
