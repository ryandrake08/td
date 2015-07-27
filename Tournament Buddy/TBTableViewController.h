//
//  TBTableViewController.h
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/31/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TBTableViewController : NSViewController

// always have a reference to the tableView and arrayController
@property (strong) IBOutlet NSTableView* tableView;
@property (strong) IBOutlet NSArrayController* arrayController;

// global configuration
@property (strong) NSMutableDictionary* configuration;

// initializer
- (instancetype)initWithNibName:(NSString*)nibName configuration:(NSMutableDictionary*)config;

@end
