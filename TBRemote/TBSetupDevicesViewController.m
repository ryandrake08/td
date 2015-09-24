//
//  TBSetupDevicesViewController.m
//  td
//
//  Created by Ryan Drake on 9/23/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDevicesViewController.h"
#import "NSDateFormatter+ISO8601.h"

@implementation TBSetupDevicesViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setArrangedObjects:[self configuration][@"authorized_clients"]];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SetupDeviceCell" forIndexPath:indexPath];
    NSDictionary* object = [super arrangedObjectForIndexPath:indexPath];
    [[cell textLabel] setText:[object[@"code"] stringValue]];
    [[cell detailTextLabel] setText:object[@"name"]];
    return cell;
}

- (id)newObject {
    NSNumber* code = @12345;
    NSString* name = @"New Device";
    NSString* added_at = [[NSDateFormatter dateFormatterWithISO8601Format] stringFromDate:[NSDate date]];

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:code, @"code", name, @"name", added_at, @"added_at", nil];
}

- (IBAction)addItem:(id)sender {
    [self addArrangedObject:[self newObject]];
}

@end
