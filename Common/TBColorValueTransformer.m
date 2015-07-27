//
//  TBColorValueTransformer.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 3/13/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBColorValueTransformer.h"
#import "TBColor+CSS.h"

@implementation TBColorValueTransformer

+ (Class)transformedValueClass {
    return [TBColor class];
}

+ (BOOL)allowsReverseTransformation {
    return YES;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        return [TBColor colorWithName:value];
    }
    return nil;
}

- (id)reverseTransformedValue:(id)value {
    return [value name];
}

+ (void)initialize {
    if (self == [TBColorValueTransformer class]) {
        TBColorValueTransformer* colorTransformer = [[TBColorValueTransformer alloc] init];
        // register it with the name that we refer to it with
        [NSValueTransformer setValueTransformer:colorTransformer
                                        forName:@"TBColorValueTransformer"];
    }
}

@end