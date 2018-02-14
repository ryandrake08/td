//
//  TBClockDateComponentsFormatter.h
//  td
//
//  Created by Ryan Drake on 1/19/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface TBClockDateComponentsFormatter : NSDateComponentsFormatter

// format clock string given number of milliseconds remaining, number of milliseconds since epoch
- (NSString*)stringFromMillisecondsRemaining:(NSNumber*)clockRemaining atMillisecondsSince1970:(NSNumber*)currentTime countingDown:(BOOL)countingDown;

@end
