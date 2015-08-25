//
//  TBMovementWindowController.h
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TBMovementWindowController : NSWindowController <NSWindowDelegate, NSTableViewDelegate>

@property (strong) IBOutlet NSArrayController* arrayController;

@end
