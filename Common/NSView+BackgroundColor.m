//
//  NSView+NSView_BackgroundColor.m
//  td
//
//  Created by Ryan Drake on 7/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "NSView+BackgroundColor.h"

@implementation NSView (NSView_BackgroundColor)

- (NSColor*)backgroundColor {
    return [NSColor colorWithCGColor:[[self layer] backgroundColor]];
}

- (void)setBackgroundColor:(NSColor*)backgroundColor {
    [self setWantsLayer:YES];
    [[self layer] setBackgroundColor:[backgroundColor CGColor]];
}

- (void)setNeedsDisplay {
    [self setNeedsDisplay:YES];
}

@end
