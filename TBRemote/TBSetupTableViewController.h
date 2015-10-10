//
//  TBSetupTableViewController.h
//  td
//
//  Created by Ryan Drake on 9/23/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBTableViewController.h"

@interface TBSetupTableViewController : TBTableViewController

// configuration being edited (set by root view controller)
@property (nonatomic, strong) NSMutableDictionary* configuration;

// specific array of items controlled by this view controller (set by self)
@property (nonatomic, strong) NSMutableArray* arrangedObjects;

// returns the object for a given indexPath
- (id)arrangedObjectForIndexPath:(NSIndexPath*)indexPath;

@end
