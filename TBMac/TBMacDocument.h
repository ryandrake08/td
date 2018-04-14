//
//  TBMacDocument.h
//  td
//
//  Created by Ryan Drake on 1/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

// Forward declare
@class TournamentSession;

@interface TBMacDocument : NSDocument

// The session controlled by the document
@property (nonatomic, strong, readonly) TournamentSession* session;

// The configuration represented by the document
@property (nonatomic, strong, readonly) NSMutableDictionary* configuration;

// Add to configuration
- (void)addConfiguration:(NSDictionary*)config;

// Add an authorized client
- (void)addAuthorizedClient:(NSDictionary*)code;

@end
