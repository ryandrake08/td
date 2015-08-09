//
//  NSView+BackgroundColor.m
//  td
//
//  Created by Ryan Drake on 7/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "NSView+BackgroundColor.h"

@implementation NSView (BackgroundColor)

- (NSColor*)backgroundColor {
    CGColorRef cgBackgroundColor = [[self layer] backgroundColor];
    if(cgBackgroundColor != NULL) {
        return [NSColor colorWithCGColor:[[self layer] backgroundColor]];
    } else {
        return nil;
    }
}

- (void)setBackgroundColor:(NSColor*)backgroundColor {
    [self setWantsLayer:YES];
    [[self layer] setBackgroundColor:[backgroundColor CGColor]];
}

- (void)setNeedsDisplay {
    [self setNeedsDisplay:YES];
}

@end
