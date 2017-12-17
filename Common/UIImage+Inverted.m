//
//  UIImage+Inverted.m
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "UIImage+Inverted.h"
#import <CoreGraphics/CoreGraphics.h>
#import <objc/runtime.h>

@implementation UIImage (Inverted)

static CGImageRef TBInvertImage(CGImageRef image)
{
    // allocate an appropriately sized buffer
    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);
    unsigned char* buffer = (unsigned char*)calloc(width*height*4, 1);

    // create bitmap context
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(buffer, width, height, 8, width * 4, colorSpace, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(colorSpace);

    // draw the current image to the newly created context
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);

    // run through every pixel, a scan line at a time...
    for(int y = 0; y < height; y++)
    {
        // get a pointer to the start of this scan line
        unsigned char* linePointer = &buffer[y * width * 4];

        // step through the pixels one by one...
        for(int x = 0; x < width; x++)
        {
            // get RGB values. We're dealing with premultiplied alpha
            // here, so we need to divide by the alpha channel (if it
            // isn't zero, of course) to get uninflected RGB. We
            // multiply by 255 to keep precision while still using
            // integers
            int r, g, b;
            if(linePointer[3])
            {
                r = linePointer[0] * 255 / linePointer[3];
                g = linePointer[1] * 255 / linePointer[3];
                b = linePointer[2] * 255 / linePointer[3];
            }
            else
                r = g = b = 0;

            // perform the colour inversion
            r = 255 - r;
            g = 255 - g;
            b = 255 - b;

            // multiply by alpha again, divide by 255 to undo the
            // scaling before, store the new values and advance
            // the pointer we're reading pixel data from
            linePointer[0] = r * linePointer[3] / 255;
            linePointer[1] = g * linePointer[3] / 255;
            linePointer[2] = b * linePointer[3] / 255;
            linePointer += 4;
        }
    }

    // get a CG image from the context
    CGImageRef returnImage = CGBitmapContextCreateImage(context);

    // clean up
    CGContextRelease(context);
    free(buffer);

    return returnImage;
}

- (UIImage*)invertedImage {
    // lookup inverted image in assosicated object
    UIImage* result = objc_getAssociatedObject(self, @selector(invertedImage));
    if(result == nil) {
        // invert
        CGImageRef outputImage = TBInvertImage([self CGImage]);
        result = [UIImage imageWithCGImage:outputImage scale:[self scale] orientation:[self imageOrientation]];
        CGImageRelease(outputImage);

        // cache in associated object
        objc_setAssociatedObject(self, @selector(invertedImage), result, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    return result;
}

@end
