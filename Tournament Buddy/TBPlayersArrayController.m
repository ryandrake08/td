//
//  TBPlayersArrayController.m
//  td
//
//  Created by Ryan Drake on 8/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBPlayersArrayController.h"

// Custom array controller, fills in a new player object
@interface TBPlayersArrayController ()

@property (strong) NSDateFormatter* dateFormatter;

@end

@implementation TBPlayersArrayController

- (id)newObject {
    if([self dateFormatter] == nil) {
        NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setLocale:[NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"]];
        [dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ss"];
        [self setDateFormatter:dateFormatter];
    }

    NSString* player_id = [[NSUUID UUID] UUIDString];
    NSString* name = @"NewPlayer";
    NSString* added_at = [[self dateFormatter] stringFromDate:[NSDate date]];

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:player_id, @"player_id", name, @"name", added_at, @"added_at", nil];
}

@end
