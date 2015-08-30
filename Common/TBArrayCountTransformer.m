//
//  TBArrayCountTransformer.m
//  td
//
//  Created by Ryan Drake on 8/30/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBArrayCountTransformer.h"

@implementation TBArrayCountTransformer

+ (Class)transformedValueClass {
    return [NSArray class];
}

+ (BOOL)allowsReverseTransformation {
    return NO;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSArray class]]) {
        return @([value count]);
    }
    return nil;
}

+ (void)initialize {
    if (self == [TBArrayCountTransformer class]) {
        TBArrayCountTransformer* arrayCountTransformer = [[TBArrayCountTransformer alloc] init];
        // register it with the name that we refer to it with
        [NSValueTransformer setValueTransformer:arrayCountTransformer
                                        forName:@"TBArrayCountTransformer"];
    }
}

@end
