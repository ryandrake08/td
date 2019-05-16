//
//  TBSetupTablesViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 5/15/19.
//  Copyright © 2019 HDna Studio. All rights reserved.
//

#import "TBSetupTablesViewController.h"

@interface TBSetupTablesViewController ()

@end

@implementation TBSetupTablesViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setArrangedObjectsKeyPath:@"configuration.available_tables"];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return [[self arrangedObjects] count];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SetupTableCell" forIndexPath:indexPath];
    NSDictionary* object = [super arrangedObjectForIndexPath:indexPath];
    [[cell textLabel] setText:object[@"table_name"]];
    return cell;
}

- (id)newObject {
    NSString* table_name = [NSString localizedStringWithFormat:@"Table %ld", (long)[[self arrangedObjects] count]+1];

    return [@{@"table_name":table_name} mutableCopy];
}

@end
