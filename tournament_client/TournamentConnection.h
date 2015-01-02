//
//  TournamentConnection.h
//  tournament_client
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface TournamentConnection : NSObject<NSStreamDelegate>

- (id)initWithHostname:(NSString*)hostname port:(UInt32)port;

@end
