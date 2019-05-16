//
//  TBSetupTablesViewController.m
//  TBMac
//
//  Created by Ryan Drake on 5/15/19.
//  Copyright © 2019 HDna Studio. All rights reserved.
//

#import "TBSetupTablesViewController.h"

// TBSetupTablesArrayController implements a new object
@interface TBSetupTablesArrayController : NSArrayController

@end

@implementation TBSetupTablesArrayController

- (id)newObject {
    NSString* table_name = [NSString localizedStringWithFormat:@"Table %ld", (long)[[self arrangedObjects] count]+1];

    return [@{@"table_name":table_name} mutableCopy];
}

@end

@interface TBSetupTablesViewController () <NSTableViewDataSource>

@end

@implementation TBSetupTablesViewController

#pragma mark NSTableViewDataSource

- (id)tableView:(NSTableView*)aTableView objectValueForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    if([[aTableColumn identifier] isEqualToString:@"Number"]) {
        return @(rowIndex+1);
    }
    return nil;
}

@end
