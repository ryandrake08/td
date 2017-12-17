//
//  TBColor+ContrastTextColor.h
//  td
//
//  Created by Ryan Drake on 12/14/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBCommon.h"

@interface TBColor (ContrastTextColor)

// YES if color should be considered perceptually dark. -contrastTextColor will be white
- (BOOL)isDark;

// white if image is dark, black otherwise
- (TBColor*)contrastTextColor;

@end
