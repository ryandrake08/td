//
//  TBSetupPlayersViewController.m
//  td
//
//  Created by Ryan Drake on 9/23/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupPlayersViewController.h"
#import "NSDateFormatter+ISO8601.h"

@implementation TBSetupPlayersViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setArrangedObjects:[self configuration][@"players"]];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SetupPlayerCell" forIndexPath:indexPath];
    NSDictionary* object = [super arrangedObjectForIndexPath:indexPath];
    [[cell textLabel] setText:object[@"name"]];
    return cell;
}

- (id)newObject {
    NSString* name = NSLocalizedString(@"New Player", nil);
    NSDateFormatter* dateFormatter = [NSDateFormatter dateFormatterWithISO8601Format];
    NSString* addedAt = [dateFormatter stringFromDate:[NSDate date]];

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:name, @"name", addedAt, @"added_at", nil];
}

@end
