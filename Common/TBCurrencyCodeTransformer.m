//
//  TBCurrencyCodeTransformer.m
//  td
//
//  Created by Ryan Drake on 8/23/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBCurrencyCodeTransformer.h"
#import "TBCurrencyNumberFormatter.h"

@interface TBCurrencyCodeTransformer ()

@property (nonatomic, strong) NSDictionary* transformer;
@property (nonatomic, strong) NSDictionary* reverseTransformer;

@end

@implementation TBCurrencyCodeTransformer

+ (Class)transformedValueClass {
    return [NSString class];
}

+ (BOOL)allowsReverseTransformation {
    return YES;
}

- (instancetype)init {
    if(self = [super init]) {
        _transformer = [NSDictionary dictionaryWithObjects:[TBCurrencyNumberFormatter supportedCurrencies] forKeys:[TBCurrencyNumberFormatter supportedCodes]];
        _reverseTransformer = [NSDictionary dictionaryWithObjects:[TBCurrencyNumberFormatter supportedCodes] forKeys:[TBCurrencyNumberFormatter supportedCurrencies]];
    }
    return self;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        return [self transformer][value];
    }
    return nil;
}

- (id)reverseTransformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        return [self reverseTransformer][value];
    }
    return nil;
}

+ (void)initialize {
    if (self == [TBCurrencyCodeTransformer class]) {
        TBCurrencyCodeTransformer* currencyTransformer = [[TBCurrencyCodeTransformer alloc] init];
        // register it with the name that we refer to it with
        [NSValueTransformer setValueTransformer:currencyTransformer
                                        forName:@"TBCurrencyCodeTransformer"];
    }
}

@end
