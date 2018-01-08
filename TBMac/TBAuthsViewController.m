//
//  TBAuthsViewController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBAuthsViewController.h"
#import "NSDateFormatter+ISO8601.h"
#import "TournamentSession.h"

// TBAuthsArrayController implements a new object
@interface TBAuthsArrayController : NSArrayController

@end

@implementation TBAuthsArrayController

- (id)newObject {
    NSNumber* code = @12345;
    NSString* name = @"New Device";
    NSString* added_at = [[NSDateFormatter dateFormatterWithISO8601Format] stringFromDate:[NSDate date]];

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:code, @"code", name, @"name", added_at, @"added_at", nil];
}

@end

@implementation TBAuthsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // filter predicate to not show this computer's auth code
    NSPredicate* predicate = [NSPredicate predicateWithFormat: @"%K != %@", @"code", [TournamentSession clientIdentifier]];
    [[self arrayController] setFilterPredicate:predicate];

    // setup sort descriptors
    NSSortDescriptor* addedAtSort = [[NSSortDescriptor alloc] initWithKey:@"added_at" ascending:YES];
    [[self arrayController] setSortDescriptors:@[addedAtSort]];
}

@end
