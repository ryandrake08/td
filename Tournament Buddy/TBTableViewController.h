//
//  TBTableViewController.h
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/31/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TBTableViewController : NSViewController

@property NSMutableDictionary* configuration;

// initializer
- (instancetype)initWithNibName:(NSString*)nibName configuration:(NSMutableDictionary*)config;

@end
