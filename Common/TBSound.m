//
//  TBSound.m
//  td
//
//  Created by Ryan Drake on 8/12/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSound.h"
#import <AudioToolbox/AudioServices.h>

@interface TBSound ()
{
    SystemSoundID handle;
}
@end

@implementation TBSound

- (instancetype)initWithContentsOfURL:(NSURL*)url {
    if((self = [super init])) {
        OSStatus errcode = AudioServicesCreateSystemSoundID((__bridge CFURLRef) url, &handle);
        if(errcode != 0) {
            NSLog(@"Failed to load sound: %@", url);
        }
    }

    return self;
}

- (instancetype)initWithContentsOfFile:(NSString*)path {
    NSURL* const url = [NSURL fileURLWithPath:path];
    return [self initWithContentsOfURL:url];
}

- (instancetype)initWithResource:(NSString*)name extension:(NSString*)extension {
    NSString* path = [[NSBundle mainBundle] pathForResource:name ofType:extension];
    return [self initWithContentsOfFile:path];
}

- (void)dealloc {
    AudioServicesDisposeSystemSoundID(handle);
}

- (void)play {
    AudioServicesPlaySystemSound(handle);
}
@end
