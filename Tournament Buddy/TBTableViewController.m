//
//  TBTableViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/31/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTableViewController.h"

@implementation TBTableViewController

// initializer
- (instancetype)initWithNibName:(NSString*)nibName configuration:(NSMutableDictionary*)config {
    self = [self initWithNibName:nibName bundle:nil];
    if (self) {
        _configuration = config;
    }
    return self;
}

@end
