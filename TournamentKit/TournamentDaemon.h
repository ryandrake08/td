//
//  TournamentDaemon.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/4/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface TournamentDaemon : NSObject

- (void)startWithService:(NSString*)service authCode:(NSNumber*)code;
- (void)stop;

@end
