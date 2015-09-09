//
//  TBAuthCodeWindowController.h
//  td
//
//  Created by Ryan Drake on 9/3/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TBAuthCodeWindowController : NSWindowController

// json representation of authorized_client
- (NSDictionary*) object;

@end
