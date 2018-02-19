//
//  TBFundingSourceValueTransformer.m
//  td
//
//  Created by Ryan Drake on 2/18/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBFundingSourceValueTransformer.h"
#import "TBFundingSourceFormatter.h"

@implementation TBFundingSourceValueTransformer

+ (Class)transformedValueClass {
    return [NSString class];
}

+ (BOOL)allowsReverseTransformation {
    return NO;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSDictionary class]]) {
        TBFundingSourceFormatter* fundingFormatter = [[TBFundingSourceFormatter alloc] init];
        return [fundingFormatter stringForObjectValue:value];
    }
    return nil;
}

+ (void)initialize {
    if (self == [TBFundingSourceValueTransformer class]) {
        TBFundingSourceValueTransformer* fundingSourceTransformer = [[TBFundingSourceValueTransformer alloc] init];
        // register it with the name that we refer to it with
        [NSValueTransformer setValueTransformer:fundingSourceTransformer
                                        forName:@"TBFundingSourceValueTransformer"];
    }
}

@end
