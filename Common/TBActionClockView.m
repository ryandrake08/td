//
//  TBActionClockView.m
//
//  Adapted from BEMAnalogClockView
//
//  Created by Boris Emorine on 2/23/14.
//  Copyright (c) 2014 Boris Emorine. All rights reserved.
//

#import "TBActionClockView.h"

CGContextRef TBGraphicsGetCurrentContext() {
#if TARGET_OS_IPHONE
    return UIGraphicsGetCurrentContext();
#else
    return [[NSGraphicsContext currentContext] graphicsPort];
#endif
}
@interface TBActionClockView () {
    double _handRadians;
}

@end

@implementation TBActionClockView

#pragma mark - Initialization

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self commonInit];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        [self commonInit];
    }
    return self;
}

- (void)commonInit {
    // Do any initialization that's common to both -initWithFrame: and -initWithCoder: in this method

    // DEFAULT VALUES
    _seconds = 0;
    _handRadians = -M_PI_2;

    _enableShadows = YES;
    _enableGraduations = YES;
    _enableDigit = YES;
    _enableArc = YES;

    _faceBackgroundColor = [TBColor grayColor];
    _faceBackgroundAlpha = 0.95;

    _borderColor = [TBColor blackColor];
    _borderAlpha = 1.0;
    _borderWidth = 3.0;

    _arcBackgroundColor = [TBColor colorWithRed:0 green:122.0/255.0 blue:255/255 alpha:1];
    _arcBackgroundAlpha = 1.0;
    _arcBorderColor = [TBColor whiteColor];
    _arcBorderAlpha = 1.0;
    _arcBorderWidth = 1.0;
    _arcFillsIn = YES;

    _handColor = [TBColor whiteColor];
    _handAlpha = 1.0;
    _handWidth = 1.0;
    _handLength = 60.0;
    _handOffsideLength = 20.0;

    _digitColor = [TBColor whiteColor];
    _digitFont  = [TBFont fontWithName:@"HelveticaNeue-Thin" size:17];
    _digitOffset = 0.0;

    self.backgroundColor = [TBColor clearColor];
}

- (void)setSeconds:(double)seconds {
    _seconds = seconds;
#if TARGET_OS_IPHONE
    _handRadians = (self.seconds * 6 * M_PI / 180) - M_PI_2;
#else
    _handRadians = (5 * M_PI_2) - (self.seconds * 6 * M_PI / 180);
#endif

    [self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect {
    // Center point
    CGPoint center = CGPointMake(self.frame.size.width / 2, self.frame.size.height / 2);

    // Face radius (out to the center of the border)
    CGFloat radius = center.x - rect.origin.x - self.borderWidth/2;

#if TARGET_OS_IPHONE
    CGFloat flip = -1.0f;
#else
    CGFloat flip = 1.0f;
#endif

    // CLOCK'S FACE
    CGContextRef ctx = TBGraphicsGetCurrentContext();
    CGContextAddEllipseInRect(ctx, rect);
    CGContextSetFillColorWithColor(ctx, self.faceBackgroundColor.CGColor);
    CGContextSetAlpha(ctx, self.faceBackgroundAlpha);
    CGContextFillPath(ctx);

    // ARC FACE
    if(self.enableArc) {
        CGContextMoveToPoint(ctx, center.x, center.y);
        CGContextAddArc(ctx, center.x, center.y, radius, flip * M_PI_2, _handRadians, self.arcFillsIn ? 1 : 0);
        CGContextSetFillColorWithColor(ctx, self.arcBackgroundColor.CGColor);
        CGContextSetAlpha(ctx, self.arcBackgroundAlpha);
        CGContextFillPath(ctx);
    }

    // CLOCK'S BORDER
    CGContextAddEllipseInRect(ctx, CGRectMake(rect.origin.x + self.borderWidth/2, rect.origin.y + self.borderWidth/2, rect.size.width - self.borderWidth, rect.size.height - self.borderWidth));
    CGContextSetStrokeColorWithColor(ctx, self.borderColor.CGColor);
    CGContextSetAlpha(ctx, self.borderAlpha);
    CGContextSetLineWidth(ctx,self.borderWidth);
    CGContextStrokePath(ctx);

    // ARC'S BORDER
    if(self.enableArc) {
        CGContextMoveToPoint(ctx, center.x, center.y);
        CGContextAddLineToPoint(ctx, center.x, center.y + flip * radius);
        CGContextAddArc(ctx, center.x, center.y, radius, flip * M_PI_2, _handRadians, self.arcFillsIn ? 1 : 0);
        CGContextAddLineToPoint(ctx, center.x, center.y);
        CGContextSetStrokeColorWithColor(ctx, self.arcBorderColor.CGColor);
        CGContextSetAlpha(ctx, self.arcBorderAlpha);
        CGContextSetLineWidth(ctx,self.borderWidth);
        CGContextStrokePath(ctx);
    }

    // CLOCK'S GRADUATION
    if (self.enableGraduations == YES) {
        TBColor* graduationColor = [TBColor whiteColor];
        CGFloat graduationAlpha = 1.0;
        CGFloat graduationWidth = 1.0;
        CGFloat graduationLength = 5.0;
        CGFloat graduationOffset = 10.0;
        
        for (int i = 0; i<60; i++) {
            if ([self.delegate respondsToSelector:@selector(analogClock:graduationColorForIndex:)]) {
                graduationColor = [self.delegate analogClock:self graduationColorForIndex:i];
            }

            if ([self.delegate respondsToSelector:@selector(analogClock:graduationAlphaForIndex:)]) {
                graduationAlpha = [self.delegate analogClock:self graduationAlphaForIndex:i];
            }

            if ([self.delegate respondsToSelector:@selector(analogClock:graduationWidthForIndex:)]) {
                graduationWidth = [self.delegate analogClock:self graduationWidthForIndex:i];
            }

            if ([self.delegate respondsToSelector:@selector(analogClock:graduationLengthForIndex:)]) {
                graduationLength = [self.delegate analogClock:self graduationLengthForIndex:i];
            }

            if ([self.delegate respondsToSelector:@selector(analogClock:graduationOffsetForIndex:)]) {
                graduationOffset = [self.delegate analogClock:self graduationOffsetForIndex:i];
            }

            CGPoint P1 = CGPointMake((center.x + ((self.frame.size.width - self.borderWidth*2 - graduationOffset) / 2) * cos((6*i)*(M_PI/180)  - (M_PI/2))), (center.x + ((self.frame.size.width - self.borderWidth*2 - graduationOffset) / 2) * sin((6*i)*(M_PI/180)  - (M_PI/2))));
            CGPoint P2 = CGPointMake((center.x + ((self.frame.size.width - self.borderWidth*2 - graduationOffset - graduationLength) / 2) * cos((6*i)*(M_PI/180)  - (M_PI/2))), (center.x + ((self.frame.size.width - self.borderWidth*2 - graduationOffset - graduationLength) / 2) * sin((6*i)*(M_PI/180)  - (M_PI/2))));

            CGContextSetStrokeColorWithColor(ctx, graduationColor.CGColor);
            CGContextSetLineWidth(ctx, graduationWidth);
            CGContextSetLineCap(ctx, kCGLineCapSquare);
            CGContextMoveToPoint(ctx, P1.x, P1.y);
            CGContextAddLineToPoint(ctx, P2.x, P2.y);
            CGContextSetBlendMode(ctx, kCGBlendModeNormal);
            CGContextSetAlpha(ctx, graduationAlpha);
            CGContextStrokePath(ctx);
        }
    }

    // HAND DRAWING
    // point that is the top of the hand (closes to the edge of the clock)
    CGPoint top = CGPointMake(center.x + self.handLength * cos(_handRadians), center.y + self.handLength * sin(_handRadians));

    // point at the bottom of the hand, a total distance offsetLength away from
    // the center of rotation.
    CGPoint bottom = CGPointMake(center.x - self.handOffsideLength * cos(_handRadians), center.y - self.handOffsideLength * sin(_handRadians));

    CGContextSetStrokeColorWithColor(ctx, self.handColor.CGColor);
    CGContextSetLineWidth(ctx, self.handWidth);
    CGContextMoveToPoint(ctx, bottom.x, bottom.y);
    CGContextAddLineToPoint(ctx, top.x, top.y);
    CGContextStrokePath(ctx);

    // DIGIT DRAWING
    if (self.enableDigit == YES) {
        CGFloat lineHeight = self.digitFont.lineHeight;
        CGFloat markingDistanceFromCenter = rect.size.width/2.0f - lineHeight/4.0f - 15 + self.digitOffset;
        NSInteger offset = 4;

        for(unsigned i = 0; i < 12; i ++){
            NSString *secondNumber = [NSString stringWithFormat:@"%@%d", i==0 ? @" ": @"", (i + 1) * 5];
            CGFloat labelX = center.x + (markingDistanceFromCenter - lineHeight/2.0f) * cos((M_PI/180) * (i+offset) * 30 + M_PI);
            CGFloat labelY = center.y + flip * (markingDistanceFromCenter - lineHeight/2.0f) * sin((M_PI/180) * (i+offset) * 30);
            CGRect rect = CGRectMake(labelX - lineHeight/2.0f, labelY - lineHeight/2.0f, lineHeight, lineHeight);
            [secondNumber drawInRect:rect withAttributes:@{NSForegroundColorAttributeName: self.digitColor, NSFontAttributeName: self.digitFont}];
        }
    }
}
@end