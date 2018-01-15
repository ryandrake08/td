//
//  TBInvertableButton.h
//  td
//
//  Created by Ryan Drake on 12/17/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface TBInvertableButton : UIButton

- (void)setImageInverted:(BOOL)inverted forState:(UIControlState)state;

@end
