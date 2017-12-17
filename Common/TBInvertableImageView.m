//
//  TBInvertableImageView.m
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBInvertableImageView.h"
#import "UIImage+Inverted.h"

@interface TBInvertableImageView ()

@property (nonatomic, strong) UIImage* originalImage;

@end

@implementation TBInvertableImageView

- (UIImage*)originalImage {
    if(_originalImage == nil) {
        // first time lookup: assume existing image is original
        [self setOriginalImage:[self image]];
    }

    return _originalImage;
}

- (void)setImageInverted:(BOOL)inverted {
    // get original image
    UIImage* original = [self originalImage];
    if(original == nil) {
        NSLog(@"TBInvertableImageView: Original image is nil");
    }

    if(inverted) {
        [self setImage:[original invertedImage]];
    } else {
        [self setImage:original];
    }
}

@end

