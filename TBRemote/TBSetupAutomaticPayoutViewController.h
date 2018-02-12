//
//  TBSetupAutomaticPayoutViewController.h
//  TBPhone
//
//  Created by Ryan Drake on 2/12/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBTableViewController.h"

@interface TBSetupAutomaticPayoutViewController : TBTableViewController

// configuration being edited (set by root view controller)
@property (nonatomic, strong) NSMutableDictionary* configuration;

@end
