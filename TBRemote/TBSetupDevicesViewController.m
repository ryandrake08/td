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

@interface TBSetupDevicesViewController ()

@property (nonatomic, strong) TBAuthCodeNumberFormatter* codeFormatter;

@end

@implementation TBSetupDevicesViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setArrangedObjects:[self configuration][@"authorized_clients"]];

    // create number formatter
    [self setCodeFormatter:[[TBAuthCodeNumberFormatter alloc] init]];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

// hide row that represents authorization for this device
- (CGFloat)tableView:(UITableView*)tableView heightForRowAtIndexPath:(NSIndexPath*)indexPath {
    NSDictionary* object = [super arrangedObjectForIndexPath:indexPath];
    if(object[@"code"] == [TournamentSession clientIdentifier]) {
        return 0;
    } else {
        return 43.5;
    }
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SetupDeviceCell" forIndexPath:indexPath];
    NSDictionary* object = [super arrangedObjectForIndexPath:indexPath];
    NSString* codeString = [[self codeFormatter] stringFromNumber:object[@"code"]];
    [[cell textLabel] setText:codeString];
    [[cell detailTextLabel] setText:object[@"name"]];
    return cell;
}

- (id)newObject {
    NSNumber* code = @12345;
    NSString* name = @"New Device";
    NSString* added_at = [[NSDateFormatter dateFormatterWithISO8601Format] stringFromDate:[NSDate date]];

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:code, @"code", name, @"name", added_at, @"added_at", nil];
}

@end
