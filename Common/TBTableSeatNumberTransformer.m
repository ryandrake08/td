//
//  TBTableSeatNumberTransformer.m
//  td
//
//  Created by Ryan Drake on 8/8/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTableSeatNumberTransformer.h"

@implementation TBTableSeatNumberTransformer

+ (Class)transformedValueClass {
    return [NSNumber class];
}

+ (BOOL)allowsReverseTransformation {
    return YES;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSNumber class]]) {
        return @([value integerValue]+1);
    }
    return nil;
}

- (id)reverseTransformedValue:(id)value {
    return @([value integerValue]-1);
}

+ (void)initialize {
    if (self == [TBTableSeatNumberTransformer class]) {
        TBTableSeatNumberTransformer* tsnTransformer = [[TBTableSeatNumberTransformer alloc] init];
        // register it with the name that we refer to it with
        [NSValueTransformer setValueTransformer:tsnTransformer
                                        forName:@"TBTableSeatNumberTransformer"];
    }
}

@end
