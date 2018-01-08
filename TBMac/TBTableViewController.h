//
//  TBTableViewController.h
//  td
//
//  Created by Ryan Drake on 1/31/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TBTableViewController : NSViewController

// always have a reference to the tableView
@property (strong) IBOutlet NSTableView* tableView;

// array controller for objects managed by this view controller
@property (strong) IBOutlet NSArrayController* arrayController;

@end
