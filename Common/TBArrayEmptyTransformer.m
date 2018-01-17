//
//  TBArrayEmptyTransformer.m
//  td
//
//  Created by Ryan Drake on 8/30/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBArrayEmptyTransformer.h"

@implementation TBArrayEmptyTransformer

+ (Class)transformedValueClass {
    return [NSArray class];
}

+ (BOOL)allowsReverseTransformation {
    return NO;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSArray class]]) {
        return @([value count] == 0);
    }
    return nil;
}

+ (void)initialize {
    if (self == [TBArrayEmptyTransformer class]) {
        TBArrayEmptyTransformer* arrayEmptyTransformer = [[TBArrayEmptyTransformer alloc] init];
        // register it with the name that we refer to it with
        [NSValueTransformer setValueTransformer:arrayEmptyTransformer
                                        forName:@"TBArrayEmptyTransformer"];
    }
}

@end
