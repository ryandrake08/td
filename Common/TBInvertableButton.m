//
//  TBInvertableButton.m
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBInvertableButton.h"
#import "UIImage+Inverted.h"

@interface TBInvertableButton ()

@property (nonatomic, strong) NSMutableDictionary* originalImages;

@end

@implementation TBInvertableButton

- (UIImage*)originalImageForState:(UIControlState)state {
    // lazy initialize cache
    if(_originalImages == nil) {
        _originalImages = [[NSMutableDictionary alloc] init];
    }

    UIImage* image = [[self originalImages] objectForKey:@(state)];
    if(image == nil) {
        // first time lookup: assume existing image is original
        image = [self imageForState:state];
        if(image != nil) {
            // add to cache as original
            [[self originalImages] setObject:image forKey:@(state)];
        }
    }

    // lookup in cache for given state
    return image;
}

- (void)setImageInverted:(BOOL)inverted forState:(UIControlState)state {
    // get original image
    UIImage* originalImage = [self originalImageForState:state];
    if(originalImage == nil) {
        NSLog(@"TBInvertableButton: Original image is nil");
    }

    if(inverted) {
        [self setImage:[originalImage invertedImage] forState:state];
    } else {
        [self setImage:originalImage forState:state];
    }
}

@end
