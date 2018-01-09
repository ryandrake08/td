//
//  TBSetupTabViewController.m
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupTabViewController.h"
#import "TBTableViewController.h"

@implementation TBSetupTabViewController

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // set represented object for each tab view
    for(id tabViewItem in [self tabViewItems]) {
        TBTableViewController* controller = (TBTableViewController*)[tabViewItem viewController];
        [controller setRepresentedObject:representedObject];
    }
}

@end
