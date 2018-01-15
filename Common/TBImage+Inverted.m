//
//  TBImage+Inverted.m
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBImage+Inverted.h"
#import "NSObject+AssociatedObject.h"

#include "TBInvertImage.h"

@implementation TBImage (Inverted)

- (TBImage*)invertedImage {
    // lookup inverted image in assosicated object
    TBImage* result = [self associatedObject];
    if(result == nil) {
        // invert
        CGImageRef outputImage = TBInvertImage([self CGImage]);
#if TARGET_OS_IPHONE
        result = [[TBImage alloc] initWithCGImage:outputImage scale:[self scale] orientation:[self imageOrientation]];
#else
        result = [[TBImage alloc] initWithCGImage:outputImage size:[self size]];
#endif
        CGImageRelease(outputImage);

        // cache in associated object
        [self setAssociatedObject:result];
    }

    return result;
}

@end
