//
//  TBRoundsArrayController.m
//  td
//
//  Created by Ryan Drake on 8/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRoundsArrayController.h"

@implementation TBRoundsArrayController

- (id)newObject {
    NSString* game_name = @"No Limit Texas Hold'em";
    NSNumber* little_blind = @25;
    NSNumber* big_blind = @50;
    NSNumber* ante = @0;
    NSNumber* duration = @3600000;

    NSDictionary* last = [[self content] lastObject];
    if(last != nil) {
        game_name = last[@"game_name"];
        little_blind = @([last[@"little_blind"] intValue] * 2);
        big_blind = @([last[@"big_blind"] intValue] * 2);
        ante = @([last[@"ante"] intValue] * 2);
        duration = last[@"duration"];
    }

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:game_name, @"game_name", little_blind, @"little_blind", big_blind, @"big_blind", ante, @"ante", duration, @"duration", nil];
}

@end
