//
//  TBCurrencyNumberFormatter.h
//  td
//
//  Created by Ryan Drake on 7/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface TBCurrencyNumberFormatter : NSNumberFormatter

+ (NSDictionary*)supportedCurrenciesForCodes;
+ (NSDictionary*)supportedCodesForCurrencies;

@end
