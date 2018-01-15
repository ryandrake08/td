//
//  TBInvertableButton.m
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBInvertableButton.h"
#import "TBImage+Inverted.h"

@interface TBInvertableButton ()

@property (nonatomic, strong) NSMutableDictionary* originalImages;

@end

@implementation TBInvertableButton

- (TBImage*)originalImageForState:(UIControlState)state {
    // lazy initialize cache
    if(_originalImages == nil) {
        _originalImages = [[NSMutableDictionary alloc] init];
    }

    // lookup in cache for given state
    TBImage* image = [[self originalImages] objectForKey:@(state)];
    if(image == nil) {
        // first time lookup: assume existing image is original
        image = [self imageForState:state];
        if(image != nil) {
            // add to cache as original
            [[self originalImages] setObject:image forKey:@(state)];
        }
    }

    return image;
}

- (void)setImageInverted:(BOOL)inverted forState:(UIControlState)state {
    // get original image
    TBImage* original = [self originalImageForState:state];
    if(original == nil) {
        NSLog(@"TBInvertableButton: Original image is nil");
    }

    if(inverted) {
        [self setImage:[original invertedImage] forState:state];
    } else {
        [self setImage:original forState:state];
    }
}

@end
