//
//  NSObject+AssociatedObject.m
//  td
//
//  Created by Ryan Drake on 8/29/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "NSObject+AssociatedObject.h"
#import <objc/runtime.h>

@implementation NSObject (AssociatedObject)

@dynamic associatedObject;

- (void)setAssociatedObject:(id)object {
    objc_setAssociatedObject(self, @selector(associatedObject), object, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

- (id)associatedObject {
    return objc_getAssociatedObject(self, @selector(associatedObject));
}

@end
