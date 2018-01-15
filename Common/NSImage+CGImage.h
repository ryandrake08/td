//
//  NSImage+CGImage.h
//  td
//
//  Created by Ryan Drake on 1/15/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>

@interface NSImage (CGImage)

// Get CGImage for NSImage
- (CGImageRef)CGImage;

@end
