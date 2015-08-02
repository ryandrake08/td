//
//  NSString+CamelCase.h
//  td
//
//  Created by Ryan Drake on 8/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSString (CamelCase)

- (NSString*)asCamelCaseFromUnderscore;
- (NSString*)asUnderscoreFromCamelCase;

@end
