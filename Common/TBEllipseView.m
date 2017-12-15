//
//  TBEllipseView.m
//  td
//
//  Created by Ryan Drake on 9/10/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBEllipseView.h"
#import "TBColor+CSS.h"
#import <CoreGraphics/CoreGraphics.h>

@implementation TBEllipseView

- (void)setColor:(UIColor*)color {
    _color = color;
    [self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect {
    // Get graphics context
    CGContextRef ctx = UIGraphicsGetCurrentContext();

    // Add colored ellipse
    CGContextAddEllipseInRect(ctx, [self bounds]);
    CGContextSetFillColorWithColor(ctx, [[self color] CGColor]);
    CGContextFillPath(ctx);

    // Draw the rest of the image
    [super drawRect:rect];
}

@end
