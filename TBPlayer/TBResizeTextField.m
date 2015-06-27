//
//  TBResizeTextField.m
//  td
//
//  Created by Ryan Drake on 6/27/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBResizeTextField.h"

@interface TBResizeTextField ()
// minimum font size to try when resizing to fit
@property IBInspectable CGFloat minFontSize;
@end

@implementation TBResizeTextField

- (void)drawRect:(NSRect)dirtyRect {
    // font to try
    NSFont* newFont, *tryFont;

    // actual rendered string dimensions
    NSSize strSize;

    // start with a small font size and go larger until rect is larger than one of the frame's dimensions
    CGFloat i = [self minFontSize];
    do {
        newFont = tryFont;
        tryFont = [NSFont fontWithName:[[self font] fontName] size:i++];
        strSize = [[self stringValue] sizeWithAttributes:[NSDictionary dictionaryWithObjectsAndKeys:tryFont, NSFontAttributeName, nil]];
    } while(strSize.width < self.frame.size.width && strSize.height < self.frame.size.height);

    if(newFont != nil) {
        // set the font
        [self setFont:newFont];
    } else {
        NSLog(@"TBResizeTextField: minFontSize %f too large for rect", [self minFontSize]);
    }

    // draw the rect otherwise as normal
    [super drawRect:dirtyRect];
}

@end
