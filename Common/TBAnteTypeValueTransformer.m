//
//  TBAnteTypeValueTransformer.m
//  TBMac
//
//  Created by Ryan Drake on 7/15/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBAnteTypeValueTransformer.h"
#import "TournamentSession.h"

@implementation TBAnteTypeValueTransformer

+ (Class)transformedValueClass {
    return [NSNumber class];
}

+ (BOOL)allowsReverseTransformation {
    return YES;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSNumber class]]) {
        return @([value isEqualToNumber:kAnteTypeBigBlind]);
    }
    return nil;
}

- (id)reverseTransformedValue:(id)value {
    return [value boolValue] ? @2 : @1;
}

+ (void)initialize {
    if (self == [TBAnteTypeValueTransformer class]) {
        TBAnteTypeValueTransformer* fundingSourceTransformer = [[TBAnteTypeValueTransformer alloc] init];
        // register it with the name that we refer to it with
        [NSValueTransformer setValueTransformer:fundingSourceTransformer
                                        forName:@"TBAnteTypeValueTransformer"];
    }
}

@end
