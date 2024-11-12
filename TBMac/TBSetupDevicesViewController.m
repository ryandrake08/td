//
//  TBSetupDevicesViewController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDevicesViewController.h"
#import "NSDateFormatter+ISO8601.h"
#import "TournamentSession.h"
#import <Foundation/Foundation.h>

// TBSetupDevicesArrayController implements a new object
@interface TBSetupDevicesArrayController : NSArrayController

@end

@implementation TBSetupDevicesArrayController

- (id)newObject {
    NSNumber* code = @12345;
    NSString* name = NSLocalizedString(@"New Device", @"Default name for a device");
    NSString* added_at = [[NSDateFormatter dateFormatterWithISO8601Format] stringFromDate:[NSDate date]];

    return [@{@"code":code, @"name":name, @"added_at":added_at} mutableCopy];
}

@end

@implementation TBSetupDevicesViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // filter predicate to not show this computer's auth code
    NSPredicate* predicate = [NSPredicate predicateWithFormat: @"code != %@", [TournamentSession clientIdentifier]];
    [[self arrayController] setFilterPredicate:predicate];

    // setup sort descriptors
    NSSortDescriptor* addedAtSort = [[NSSortDescriptor alloc] initWithKey:@"added_at" ascending:YES];
    [[self arrayController] setSortDescriptors:@[addedAtSort]];
}

@end
