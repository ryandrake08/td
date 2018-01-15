//
//  TBInvertableButton.m
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBInvertableButton_iOS.h"
#import "TBImage+Inverted.h"

@interface TBInvertableButton ()

@property (nonatomic, strong) UIImage* originalNormalImage;
@property (nonatomic, strong) UIImage* originalHighlightedImage;

@end

@implementation TBInvertableButton

- (instancetype)initWithCoder:(NSCoder *)coder {
    if(self = [super initWithCoder:coder]) {
        _originalNormalImage = [self imageForState:UIControlStateNormal];
        _originalHighlightedImage = [self imageForState:UIControlStateHighlighted];
    }
    return self;
}

- (TBImage*)originalImageForState:(UIControlState)state {
    TBImage* image = nil;
    switch(state) {
        case UIControlStateNormal:
            image = _originalNormalImage;
            break;
        case UIControlStateHighlighted:
            image = _originalHighlightedImage;
            break;
        default:
            NSLog(@"TBInvertableButton only supports UIControlStateNormal and UIControlStateHighlighted for now");
            break;
    }
    return image;
}

- (void)setImageInverted:(BOOL)inverted forState:(UIControlState)state {
    TBImage* original = [self originalImageForState:state];
    if(inverted) {
        [self setImage:[original invertedImage] forState:state];
    } else {
        [self setImage:original forState:state];
    }
}

@end
