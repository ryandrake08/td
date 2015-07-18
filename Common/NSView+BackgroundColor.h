//
//  NSView+NSView_BackgroundColor.h
//  td
//
//  Created by Ryan Drake on 7/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSView (NSView_BackgroundColor)

@property NSColor* backgroundColor;

- (void)setNeedsDisplay;

@end
