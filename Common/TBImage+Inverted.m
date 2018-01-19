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
#if TARGET_OS_IPHONE
        CGImageRef outputImage = TBCreateInvertedImage([self CGImage]);
        result = [[TBImage alloc] initWithCGImage:outputImage scale:[self scale] orientation:[self imageOrientation]];
#else
        // NSImage does not have -[CGImage] and no straightforward way to make a category on it in ARC that returns an autoreleased CFImage so...
        CGImageSourceRef source = CGImageSourceCreateWithData((CFDataRef)[self TIFFRepresentation], NULL);
        CGImageRef imageRef = CGImageSourceCreateImageAtIndex(source, 0, NULL);
        CFRelease(source);
        CGImageRef outputImage = TBCreateInvertedImage(imageRef);
        CGImageRelease(imageRef);
        result = [[TBImage alloc] initWithCGImage:outputImage size:[self size]];
#endif
        CGImageRelease(outputImage);

        // cache in associated object
        [self setAssociatedObject:result];
    }

    return result;
}

@end
