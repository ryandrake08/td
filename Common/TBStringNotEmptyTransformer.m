//
//  TBStringNotEmptyTransformer.m
//  TBMac
//
//  Created by Ryan Drake on 10/16/21.
//  Copyright © 2021 HDna Studio. All rights reserved.
//

#import "TBStringNotEmptyTransformer.h"

@implementation TBStringNotEmptyTransformer

+ (Class)transformedValueClass {
    return [NSString class];
}

+ (BOOL)allowsReverseTransformation {
    return YES;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        return @([value length] != 0);
    }
    return nil;
}

- (id)reverseTransformedValue:(id)value {
    return nil;
}


+ (void)initialize {
    if (self == [TBStringNotEmptyTransformer class]) {
        TBStringNotEmptyTransformer* stringNotEmptyTransformer = [[TBStringNotEmptyTransformer alloc] init];
        // register it with the name that we refer to it with
        [NSValueTransformer setValueTransformer:stringNotEmptyTransformer
                                        forName:@"TBStringNotEmptyTransformer"];
    }
}

@end
