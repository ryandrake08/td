//
//  TBLeagueViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBLeagueViewController.h"
#import "NSDateFormatter+ISO8601.h"

// TBLeagueArrayController implements a new object
@interface TBLeagueArrayController : NSArrayController

@end

@implementation TBLeagueArrayController

- (id)newObject {
    NSString* player_id = [[NSUUID UUID] UUIDString];
    NSString* name = @"[New Player Name]";
    NSString* added_at = [[NSDateFormatter dateFormatterWithISO8601Format] stringFromDate:[NSDate date]];

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:player_id, @"player_id", name, @"name", added_at, @"added_at", nil];
}

@end

@implementation TBLeagueViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* nameSort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES];
    [[self arrayController] setSortDescriptors:@[nameSort]];
}

@end
