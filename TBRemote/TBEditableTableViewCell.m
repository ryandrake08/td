//
//  TBEditableTableViewCell.m
//  td
//
//  Created by Ryan Drake on 9/25/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBEditableTableViewCell.h"
#include "NSObject+AssociatedObject.h"

@interface TBEditableTableViewCell () <UITextFieldDelegate, UIPickerViewDelegate, UIPickerViewDataSource>

// ui outlet
@property (nonatomic, strong) IBOutlet UITextField* textField;

// keypath of object to be edited
@property (nonatomic, copy) NSString* keyPath;

// allowed values (picker)
@property (nonatomic, copy) NSArray* allowedValues;

// titles for each allowed value (picker)
@property (nonatomic, copy) NSArray* allowedValueTitles;

@end

@implementation TBEditableTableViewCell

// set the object and keypath to observe/sync with text field
- (void)setEditableObject:(id)object keypath:(NSString*)keyPath {
    [self setAssociatedObject:object];
    [self setKeyPath:keyPath];

    NSString* text = [self textValueForObject:object[keyPath]];
    [[self textField] setText:text];
}

// default textValueForObject
- (NSString*)textValueForObject:(id)object {
    if([object isKindOfClass:[NSString class]]) {
        return [object copy];
    } else if([object isKindOfClass:[NSNumber class]] && [self formatter] != nil) {
        return [[self formatter] stringFromNumber:object];
    }
    return nil;
}

// default objectForTextValue
- (id)objectForTextValue:(NSString*)text {
    if([self formatter] == nil) {
        return [text copy];
    } else {
        return [[self formatter] numberFromString:text];
    }
}

// when selected, make the edit field become the first responder
- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];
    if(selected) {
        [[self textField] becomeFirstResponder];
    }
}

// use a picker instead of free-form text
- (void)setAllowedValues:(NSArray*)allowedValues withTitles:(NSArray*)titles {
    // reset picker
    [[self textField] setInputView:nil];
    [[self textField] setInputAccessoryView:nil];

    if(allowedValues) {
        // get text field's current object value
        NSUInteger selectedIndex;
        if(titles) {
            selectedIndex = [titles indexOfObject:[[self textField] text]];
        } else {
            id value = [self objectForTextValue:[[self textField] text]];
            selectedIndex = [allowedValues indexOfObject:value];
        }

        // is current text value allowed? if so, select it, otherwise select first object
        if(selectedIndex == NSNotFound) {
            selectedIndex = 0;
        }

        // set state
        [self setAllowedValues:allowedValues];
        [self setAllowedValueTitles:titles];

        // set up a picker
        UIPickerView* picker = [[UIPickerView alloc] init];
        [picker setDataSource:self];
        [picker setDelegate:self];
        [picker selectRow:selectedIndex inComponent:0 animated:NO];
        [[self textField] setInputView:picker];

        // set up picker tool bar
        UIToolbar* toolbar = [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, 320, 44)];
        UIBarButtonItem* space = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil];
        UIBarButtonItem* doneButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:[self textField] action:@selector(resignFirstResponder)];
        [toolbar setItems:@[space, doneButton]];
        [[self textField] setInputAccessoryView:toolbar];
    }
}

#pragma mark UITextFieldDelegate

- (BOOL)textFieldShouldReturn:(UITextField*)textField {
    [textField resignFirstResponder];
    return YES;
}

- (void)textFieldDidEndEditing:(UITextField*)textField {
    id value = [self objectForTextValue:[textField text]];
    [[self associatedObject] setValue:value forKeyPath:[self keyPath]];
}

#pragma mark UIPickerViewDataSource

- (NSInteger)pickerView:(UIPickerView*)pickerView numberOfRowsInComponent:(NSInteger)component {
    return [[self allowedValues] count];
}

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView*)pickerView {
    return 1;
}

- (NSString*)pickerView:(UIPickerView*)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component {
    if([self allowedValueTitles]) {
        return [self allowedValueTitles][row];
    } else {
        return [self textValueForObject:[self allowedValues][row]];
    }
}

#pragma mark UIPickerViewDelegate

- (void)pickerView:(UIPickerView*)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component {
    NSString* text;
    if([self allowedValueTitles]) {
        text = [self allowedValueTitles][row];
    } else {
        text = [self textValueForObject:[self allowedValues][row]];
    }
    [[self textField] setText:text];
}

@end

@implementation TBEditableNumberTableViewCell

- (void)awakeFromNib {
    [self setFormatter:[[NSNumberFormatter alloc] init]];
}

@end