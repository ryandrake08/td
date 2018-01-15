//
//  TBGraphics.h
//  td
//
//  Created by Ryan Drake on 1/15/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>

// Get the current graphics context. Different implementation on macOS vs. iOS
CGContextRef TBGraphicsGetCurrentContext(void);
