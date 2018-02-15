//
//  TBSetupPlayersViewController.m
//  td
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSetupPlayersViewController.h"
#import "NSDateFormatter+ISO8601.h"

// TBSetupPlayersArrayController implements a new object
@interface TBSetupPlayersArrayController : NSArrayController

@end

@implementation TBSetupPlayersArrayController

- (id)newObject {
    NSString* player_id = [[NSUUID UUID] UUIDString];
    NSString* name = NSLocalizedString(@"Player Name", @"Default name for a player");
    NSString* added_at = [[NSDateFormatter dateFormatterWithISO8601Format] stringFromDate:[NSDate date]];

    return [@{@"player_id":player_id, @"name":name, @"added_at":added_at} mutableCopy];
}

@end

@implementation TBSetupPlayersViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* nameSort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES];
    [[self arrayController] setSortDescriptors:@[nameSort]];
}

@end
