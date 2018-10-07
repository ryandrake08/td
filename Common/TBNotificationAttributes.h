//
//  TBNotificationAttributes.h
//  td
//
//  Created by Ryan Drake on 10/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface TBNotificationAttributes : NSObject

@property (nonatomic, copy) NSString* title;
@property (nonatomic, copy) NSString* body;
@property (nonatomic, copy) NSString* soundName;
@property (nonatomic, copy) NSDate* date;

// initialize this object from a tournament state
- (instancetype)initWithTournamentState:(NSDictionary*)state warningTime:(NSInteger)warningTime;

@end
