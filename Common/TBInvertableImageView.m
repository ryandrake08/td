//
//  TBInvertableImageView.m
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBInvertableImageView.h"
#import "TBImage+Inverted.h"

@interface TBInvertableImageView ()

@property (nonatomic, strong) TBImage* originalImage;

@end

@implementation TBInvertableImageView

- (instancetype)initWithCoder:(NSCoder *)coder {
    if(self = [super initWithCoder:coder]) {
        _originalImage = [self image];
    }
    return self;
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

@end

