//
//  TBEditableTextTableViewCell.m
//  td
//
//  Created by Ryan Drake on 9/25/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBEditableTableViewCell.h"

@interface TBEditableTableViewCell ()

// object (KV observable) to be edited
@property (nonatomic, retain) id object;

// keypath of object to be edited
@property (nonatomic, copy) NSString* keyPath;

@end

@implementation TBEditableTableViewCell

// set the object and keypath to observe/sync with text field
- (void)setEditableObject:(id)object keypath:(NSString*)keyPath {
    [self setObject:object];
    [self setKeyPath:keyPath];
}

@end

@interface TBEditableTextTableViewCell () <UITextFieldDelegate>

// ui outlet
@property (nonatomic, strong) IBOutlet UITextField* textField;

@end

@implementation TBEditableTextTableViewCell

// set the object and keypath to observe/sync with text field
- (void)setEditableObject:(id)object keypath:(NSString*)keyPath {
    [super setEditableObject:object keypath:keyPath];
    [self updateTextField];
}

// update the text field to match the editable object
- (void)updateTextField {
    id object = [self object][[self keyPath]];
    if([object isKindOfClass:[NSString class]]) {
        [[self textField] setText:object];
    } else {
        NSLog(@"TBEditableTextTableViewCell: editable object is not a string");
    }
}

// update the editable object to match the text field
- (void)updateEditableObject {
    [self object][[self keyPath]] = [[self textField] text];
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
    [self updateEditableObject];
}

@end

@implementation TBEditableNumberTableViewCell

- (void)awakeFromNib {
    [self setFormatter:[[NSNumberFormatter alloc] init]];
}

// update the text field to match the editable object
- (void)updateTextField {
    id object = [self object][[self keyPath]];
    if([object isKindOfClass:[NSNumber class]]) {
        NSString* text;
        if([self formatter] != nil) {
            text = [[self formatter] stringFromNumber:object];
        } else {
            text = [object stringValue];
        }
        [[self textField] setText:text];
    } else {
        NSLog(@"TBEditableNumberTableViewCell: editable object is not a number");
    }
}

// update the editable object to match the text field
- (void)updateEditableObject {
    NSString* text = [[self textField] text];
    NSNumber* number;
    if([self formatter] != nil) {
        number = [[self formatter] numberFromString:text];
    } else {
        number = [NSNumber numberWithDouble:[text doubleValue]];
    }
    [self object][[self keyPath]] = number;
}

@end

@interface TBPickableTextTableViewCell () <UIPickerViewDelegate, UIPickerViewDataSource>

// allowed values (picker)
@property (nonatomic, copy) NSArray* allowedValues;

// titles for each allowed value (picker)
@property (nonatomic, copy) NSArray* allowedValueTitles;

@end

@implementation TBPickableTextTableViewCell

// update the text field to match the editable object
- (void)updateTextField {
    id object = [self object][[self keyPath]];
    if([self allowedValueTitles]) {
        NSUInteger index = [[self allowedValues] indexOfObject:object];
        if(index == NSNotFound) {
            NSLog(@"TBPickableTextTableViewCell: object not found in allowed list. Using first title");
            index = 0;
        }
        [[self textField] setText:[self allowedValueTitles][index]];
    } else {
        NSLog(@"TBPickableTextTableViewCell: no titles");
    }
}

// update the editable object to match the text field
- (void)updateEditableObject {
    NSString* text = [[self textField] text];
    if([self allowedValueTitles]) {
        NSUInteger index = [[self allowedValueTitles] indexOfObject:text];
        if(index == NSNotFound) {
            NSLog(@"TBPickableTextTableViewCell: title not found in allowed list. Using first object");
            index = 0;
        }
        [self object][[self keyPath]] = [self allowedValues][index];
    } else {
        NSLog(@"TBPickableTextTableViewCell: no titles");
    }
}

// use a picker instead of free-form text
- (void)setAllowedValues:(NSArray*)allowedValues withTitles:(NSArray*)titles {
    // reset picker
    [[self textField] setInputView:nil];
    [[self textField] setInputAccessoryView:nil];

    // build default titles if missing
    if(titles == nil) {
        titles = [allowedValues valueForKey:@"stringValue"];
    }

    // set state
    [self setAllowedValues:allowedValues];
    [self setAllowedValueTitles:titles];

    if(allowedValues) {
        // get text field's current object value
        NSUInteger selectedIndex = [titles indexOfObject:[[self textField] text]];

        // is current text value allowed? if so, select it, otherwise select first object
        if(selectedIndex == NSNotFound) {
            selectedIndex = 0;
        }

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

#pragma mark UIPickerViewDataSource

- (NSInteger)pickerView:(UIPickerView*)pickerView numberOfRowsInComponent:(NSInteger)component {
    return [[self allowedValues] count];
}

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView*)pickerView {
    return 1;
}

- (NSString*)pickerView:(UIPickerView*)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component {
    return [self allowedValueTitles][row];
}

#pragma mark UIPickerViewDelegate

- (void)pickerView:(UIPickerView*)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component {
    [[self textField] setText:[self allowedValueTitles][row]];
}

@end