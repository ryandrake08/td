//
//  TBSetupPayoutsViewController.m
//  TBMac
//
//  Created by Ryan Drake on 3/10/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupPayoutsViewController.h"

@interface TBSetupPayoutsViewController ()

@end

@implementation TBSetupPayoutsViewController

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // set represented object for each tab view
    for(id tabViewItem in [self tabViewItems]) {
        NSViewController* controller = [tabViewItem viewController];
        [controller setRepresentedObject:representedObject];
    }
}

@end
