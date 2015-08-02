//
//  TBColor+CSS.h
//  TournamentKit
//
//  Created by Ryan Drake on 2/17/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBCommon.h"

@interface TBColor (CSS)

// Return a random color
+ (NSString*)randomColorName;

// Lookup a color using css 3/svg color name, or RGB hex
+ (TBColor*)colorWithName:(NSString*)colorName;

// Get a css 3/svg color name for color
- (NSString*)name;

@end
