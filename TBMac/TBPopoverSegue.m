//
//  TBPopoverSegue.m
//  TBMac
//
//  Created by Ryan Drake on 2/18/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBPopoverSegue.h"

@implementation TBPopoverSegue

- (void)perform {
    if ([self anchorView]) {
        // Use the presentation API so that the popover can be dismissed using -dismissController:.
        [[self sourceController] presentViewController:[self destinationController] asPopoverRelativeToRect:[[self anchorView] bounds] ofView:[self anchorView] preferredEdge:[self preferredEdge] behavior:[self popoverBehavior]];
    }
}

@end
