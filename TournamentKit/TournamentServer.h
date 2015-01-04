//
//  TournamentServer.h
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

#define kDefaultTournamentServerPort 25600

@interface TournamentServer : NSObject

@property (nonatomic, copy) NSString* name;
@property (nonatomic, copy) NSString* address;
@property (nonatomic, assign) NSInteger port;
@property (nonatomic, assign) BOOL manuallyAdded;

@end
