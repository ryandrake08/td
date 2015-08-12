//
//  TBSound.h
//  td
//
//  Created by Ryan Drake on 8/12/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface TBSound : NSObject

- (instancetype)initWithContentsOfURL:(NSURL*)url;
- (instancetype)initWithContentsOfFile:(NSString*)path;
- (instancetype)initWithResource:(NSString*)name extension:(NSString*)extension;
- (void)play;

@end
