//
//  TBPopoverSegue.h
//  TBMac
//
//  Created by Ryan Drake on 2/18/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TBPopoverSegue : NSStoryboardSegue

@property (weak) NSView* anchorView;
@property NSRectEdge preferredEdge;
@property NSPopoverBehavior popoverBehavior;

@end
