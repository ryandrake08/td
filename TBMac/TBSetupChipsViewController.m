//
//  TBSetupChipsViewController.m
//  td
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSetupChipsViewController.h"
#import "TBColor+CSS.h"

// TBSetupChipsArrayController implements a new object
@interface TBSetupChipsArrayController : NSArrayController

@end

@implementation TBSetupChipsArrayController

- (id)newObject {
    NSString* color = [TBColor randomColorName];
    NSNumber* denomination = @1;
    NSNumber* count_available = @100;

    return [@{@"color":color, @"denomination":denomination, @"count_available":count_available} mutableCopy];
}

@end

@implementation TBSetupChipsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* denominationSort = [[NSSortDescriptor alloc] initWithKey:@"denomination" ascending:YES];
    [[self arrayController] setSortDescriptors:@[denominationSort]];
}

@end
