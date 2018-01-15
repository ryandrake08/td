//
//  TBImage+Inverted.m
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBImage+Inverted.h"
#import <objc/runtime.h>

#include "TBInvertImage.h"

@implementation TBImage (Inverted)

- (TBImage*)invertedImage {
    // lookup inverted image in assosicated object
    TBImage* result = objc_getAssociatedObject(self, @selector(invertedImage));
    if(result == nil) {
        // invert
        CGImageRef outputImage = TBInvertImage([self CGImage]);
#if TARGET_OS_IPHONE
        result = [TBImage imageWithCGImage:outputImage scale:[self scale] orientation:[self imageOrientation]];
#else
        result = [[TBImage alloc] initWithCGImage:outputImage size:[self size]];
#endif
        CGImageRelease(outputImage);

        // cache in associated object
        objc_setAssociatedObject(self, @selector(invertedImage), result, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    return result;
}

@end
