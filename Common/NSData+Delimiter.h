//
//  NSData+Delimiter.h
//  td
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSData (Delimiter)

- (NSRange)rangeOfDataDelimitedBy:(uint8_t)delimiter;
- (NSRange)rangeOfDataDelimetedBy:(uint8_t)delimiter range:(NSRange)searchRange;

@end
