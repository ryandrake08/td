//
//  TBInvertableButton.h
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TBInvertableButton : NSButton

- (void)setImageInverted:(BOOL)inverted;
- (void)setAlternateImageInverted:(BOOL)inverted;

@end
