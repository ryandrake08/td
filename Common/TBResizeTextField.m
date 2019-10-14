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
@property (nonatomic, assign) IBInspectable CGFloat minFontSize;

// cut down on transient memory allocations by keeping font/size combos in this cache
@property (nonatomic, strong) NSMutableArray* fontAttributesCache;

@end

@implementation TBResizeTextField

- (void)resizeFont {
    // font to try
    NSFont* newFont, *tryFont;

    // actual rendered string dimensions
    NSSize strSize;

    // create the cache if missing
    if([self fontAttributesCache] == nil) {
        [self setFontAttributesCache:[[NSMutableArray alloc] init]];
    }

    // start with a small font size and go larger until rect is larger than one of the frame's dimensions
    CGFloat i = [self minFontSize];
    do {
        NSDictionary* tryFontAttributes;

        // use the previous font, which fit into the frame. try the next higher size
        newFont = tryFont;

        // look up in our cache first
        NSUInteger cacheIndex = (NSUInteger)(i - [self minFontSize]);
        if([[self fontAttributesCache] count] > cacheIndex) {
            // font exists in our cache, select it
            tryFontAttributes = [self fontAttributesCache][cacheIndex];
            tryFont = tryFontAttributes[NSFontAttributeName];
        } else {
            // allocate a new font
            tryFont = [NSFont fontWithName:[[self font] fontName] size:i];
            tryFontAttributes = @{NSFontAttributeName:tryFont};
            // insert into cache
            [[self fontAttributesCache] insertObject:tryFontAttributes atIndex:cacheIndex];
        }

        // measure the selected font
        strSize = [[self stringValue] sizeWithAttributes:tryFontAttributes];

        // increment the size for possible next iteration
        i++;
    } while(strSize.width < self.frame.size.width && strSize.height < self.frame.size.height);

    if(newFont != nil) {
        // set the font
        [self setFont:newFont];
    } else {
        NSLog(@"TBResizeTextField: minFontSize %f too large for rect %@", [self minFontSize], NSStringFromSize(NSSizeFromCGSize(self.frame.size)));
    }
}

- (void)setObjectValue:(id)objectValue {
    if(![objectValue isEqual:[self objectValue]]) {
        [super setObjectValue:objectValue];
        [self resizeFont];
    }
}

- (void)setFrame:(NSRect)frame {
    if(!CGRectEqualToRect(frame, [self frame])) {
        [super setFrame:frame];
        [self resizeFont];
    }
}

@end
