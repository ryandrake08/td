//
//  TBEditableTableViewCell.m
//  td
//
//  Created by Ryan Drake on 9/25/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBEditableTableViewCell.h"
#include "NSObject+FBKVOController.h"

@interface TBEditableTableViewCell () <UITextFieldDelegate>

// ui outlet
@property (nonatomic, strong) IBOutlet UITextField* textField;

// object to be edited
@property (nonatomic, strong) id object;

// keypath of object to be edited
@property (nonatomic, copy) NSString* keyPath;

@end

@implementation TBEditableTableViewCell

// set the object and keypath to observe/sync with text field
- (void)setEditableObject:(id)object keypath:(NSString *)keyPath {
    [self setObject:object];
    [self setKeyPath:keyPath];
    [[[self textField] KVOController] unobserveAll];
    [[[self textField] KVOController] observe:object keyPath:keyPath options:NSKeyValueObservingOptionNew block:^(id observer, id object, NSDictionary* change) {
        NSString* text = [self textValueForObject:change[@"new"]];
        [observer setText:text];
    }];
}

// default textValueForObject
- (NSString*)textValueForObject:(id)object {
    if([object isKindOfClass:[NSString class]]) {
        return [object copy];
    }
    return nil;
}

// default objectForTextValue
- (id)objectForTextValue:(NSString*)text {
    return [text copy];
}

// when selected, make the edit field become the first responder
- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];
    if(selected) {
        [[self textField] becomeFirstResponder];
    }
}

#pragma mark UITextFieldDelegate

- (BOOL)textFieldShouldReturn:(UITextField*)textField {
    [textField resignFirstResponder];
    return YES;
}

- (void)textFieldDidEndEditing:(UITextField*)textField {
    id value = [self objectForTextValue:[textField text]];
    [[self object] setValue:value forKeyPath:[self keyPath]];
}

@end

@implementation TBEditableNumberTableViewCell

// default textValueForObject
- (NSString*)textValueForObject:(id)object {
    if([object isKindOfClass:[NSNumber class]]) {
        return [[self formatter] stringFromNumber:object];
    }
    return nil;
}

// default objectForTextValue
- (id)objectForTextValue:(NSString*)text {
    return [[self formatter] numberFromString:text];
}

@end