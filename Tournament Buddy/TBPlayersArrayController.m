//
//  TBPlayersArrayController.m
//  td
//
//  Created by Ryan Drake on 8/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBPlayersArrayController.h"
#import "NSDateFormatter+ISO8601.h"

// Custom array controller, fills in a new player object
@interface TBPlayersArrayController ()

@end

@implementation TBPlayersArrayController

- (id)newObject {
    NSString* player_id = [[NSUUID UUID] UUIDString];
    NSString* name = @"[New Player Name]";
    NSString* added_at = [[NSDateFormatter dateFormatterWithISO8601Format] stringFromDate:[NSDate date]];

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:player_id, @"player_id", name, @"name", added_at, @"added_at", nil];
}

@end
