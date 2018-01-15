//
//  TBInvertableButton.m
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBInvertableButton_macOS.h"
#import "TBImage+Inverted.h"

@interface TBInvertableButton ()

@property (nonatomic, strong) TBImage* originalImage;
@property (nonatomic, strong) TBImage* originalAlternateImage;

@end

@implementation TBInvertableButton

- (instancetype)initWithCoder:(NSCoder *)coder {
    if(self = [super initWithCoder:coder]) {
        _originalImage = [self image];
        _originalAlternateImage = [self alternateImage];
    }
    return self;
}

- (TBImage*)originalImage {
    return _originalImage;
}

- (BOOL)imageInverted {
    return ![[self image] isEqual:[self originalImage]];
}

- (void)setImageInverted:(BOOL)inverted {
    if(inverted) {
        [self setImage:[[self originalImage] invertedImage]];
    } else {
        [self setImage:[self originalImage]];
    }
}

- (TBImage*)originalAlternateImage {
    return _originalAlternateImage;
}

- (BOOL)alternateImageInverted {
    return ![[self alternateImage] isEqual:[self originalAlternateImage]];
}

- (void)setAlternateImageInverted:(BOOL)inverted {
    if(inverted) {
        [self setAlternateImage:[[self originalAlternateImage] invertedImage]];
    } else {
        [self setAlternateImage:[self originalAlternateImage]];
    }
}

@end
