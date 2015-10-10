//
//  TBSetupDetialsTableViewController.h
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBTableViewController.h"

@interface TBSetupDetailsTableViewController : TBTableViewController

// configuration being edited
@property (nonatomic, strong) NSMutableDictionary* configuration;

// object being edited
@property (nonatomic, strong) NSMutableDictionary* object;

@end
