//
//  UIImage+Inverted.m
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "UIImage+Inverted.h"
#import <CoreImage/CoreImage.h>
#import <objc/runtime.h>

@implementation UIImage (Inverted)

- (UIImage*)invertedImage {
    // lookup inverted image in assosicated object
    UIImage* result = objc_getAssociatedObject(self, @selector(invertedImage));
    if(result == nil) {
        // invert
        CIImage* original = [CIImage imageWithCGImage:[self CGImage]];
        CGRect rect = [original extent];
        CIFilter* filter = [CIFilter filterWithName:@"CIColorInvert"];
        [filter setDefaults];
        [filter setValue:original forKey:kCIInputImageKey];
        CIImage* output = [filter outputImage];
        CIContext* context = [CIContext new];
        CGImageRef outputImage = [context createCGImage:output fromRect:rect];
        result = [UIImage imageWithCGImage:outputImage scale:[self scale] orientation:[self imageOrientation]];

        // cache in associated object
        objc_setAssociatedObject(self, @selector(invertedImage), result, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    return result;
}

@end
