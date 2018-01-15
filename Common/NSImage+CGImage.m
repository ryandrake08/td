//
//  NSImage+CGImage.m
//  td
//
//  Created by Ryan Drake on 1/15/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "NSImage+CGImage.h"

@implementation NSImage (CGImage)

- (CGImageRef)CGImage {
    CGImageSourceRef source = CGImageSourceCreateWithData((CFDataRef)[self TIFFRepresentation], NULL);
    CGImageRef imageRef = CGImageSourceCreateImageAtIndex(source, 0, NULL);
    CFRelease(source);
    return imageRef;
}

@end
