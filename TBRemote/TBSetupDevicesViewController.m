//
//  TBSetupDevicesViewController.m
//  td
//
//  Created by Ryan Drake on 9/23/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDevicesViewController.h"
#import "TBAuthCodeNumberFormatter.h"
#import "NSDateFormatter+ISO8601.h"
#import "TournamentSession.h"

@implementation TBSetupDevicesViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setArrangedObjectsKeyPath:@"configuration.authorized_clients"];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SetupDeviceCell" forIndexPath:indexPath];
    NSDictionary* object = [super arrangedObjectForIndexPath:indexPath];
    TBAuthCodeNumberFormatter* codeFormatter = [[TBAuthCodeNumberFormatter alloc] init];
    NSString* codeString = [codeFormatter stringFromNumber:object[@"code"]];
    [[cell textLabel] setText:codeString];
    [[cell detailTextLabel] setText:object[@"name"]];
    return cell;
}

- (id)newObject {
    NSNumber* code = @12345;
    NSString* name = NSLocalizedString(@"New Device", nil);
    NSString* added_at = [[NSDateFormatter dateFormatterWithISO8601Format] stringFromDate:[NSDate date]];

    return [@{@"code":code, @"name":name, @"added_at":added_at} mutableCopy];
}

@end
