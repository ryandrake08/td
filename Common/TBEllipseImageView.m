//
//  TBEllipseImageView.m
//  td
//
//  Created by Ryan Drake on 9/10/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBEllipseImageView.h"

@implementation TBEllipseImageView

- (void)drawRect:(CGRect)rect {
    // Get graphics context
#if TARGET_OS_IPHONE
    CGContextRef ctx = UIGraphicsGetCurrentContext();
#else
    CGContextRef ctx = [[NSGraphicsContext currentContext] graphicsPort];
#endif

    // Full image rect (not necessarily the draw rect)
    CGRect imageRect = CGRectMake(0.0f, 0.0f, self.image.size.width, self.image.size.height);

    // Add colored ellipse
    CGContextAddEllipseInRect(ctx, imageRect);
    CGContextSetFillColorWithColor(ctx, self.color.CGColor);
    CGContextFillPath(ctx);

    // Draw the rest of the image
    [super drawRect:rect];
}

@end
