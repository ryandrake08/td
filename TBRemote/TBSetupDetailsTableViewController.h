//
//  TBSetupDetialsTableViewController.h
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBTableViewController.h"

@interface TBSetupDetailsTableViewController : TBTableViewController

// object being edited
@property (nonatomic, strong) NSMutableDictionary* object;

// pass object to cell if supported
- (UITableViewCell*)setObjectToCell:(UITableViewCell*)cell;

@end
