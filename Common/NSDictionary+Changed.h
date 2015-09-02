//
//  NSDictionary+Changed.h
//  td
//
//  Created by Ryan Drake on 9/1/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSDictionary (Changed)

- (NSDictionary*)dictionaryWithChangesFromDictionary:(NSDictionary*)other;

@end
