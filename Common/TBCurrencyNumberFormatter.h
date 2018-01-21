//
//  TBCurrencyNumberFormatter.h
//  td
//
//  Created by Ryan Drake on 7/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface TBCurrencyNumberFormatter : NSNumberFormatter

@property (nonatomic, copy, readonly) NSArray* supportedCurrencies;
@property (nonatomic, copy, readonly) NSArray* supportedCodes;

+ (NSArray*)supportedCurrencies;
+ (NSArray*)supportedCodes;
+ (NSDictionary*)supportedCurrenciesForCodes;
+ (NSDictionary*)supportedCodesForCurrencies;

@end
