//
//  TBChipsViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBChipsViewController.h"
#import "TBColor+CSS.h"

// TBChipsArrayController implements a new object
@interface TBChipsArrayController : NSArrayController

@end

@implementation TBChipsArrayController

- (id)newObject {
    NSString* color = [TBColor randomColorName];
    NSNumber* denomination = @1;
    NSNumber* count_available = @100;

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:color, @"color", denomination, @"denomination", count_available, @"count_available", nil];
}

@end

@implementation TBChipsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* denominationSort = [[NSSortDescriptor alloc] initWithKey:@"denomination" ascending:YES];
    [[self arrayController] setSortDescriptors:@[denominationSort]];
}

@end
