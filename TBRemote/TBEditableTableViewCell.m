//
//  TBEditableTextTableViewCell.m
//  td
//
//  Created by Ryan Drake on 9/25/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBEditableTableViewCell.h"
#import "TBNotifications.h"

@implementation TBEditableTableViewCell
@end

@interface TBEditableTextTableViewCell () <UITextFieldDelegate>

// ui outlet
@property (nonatomic, strong) IBOutlet UITextField* textField;

@end

@implementation TBEditableTextTableViewCell

// set the object and keypath to observe/sync with text field
- (void)setKeyPath:(NSString*)keyPath {
    [super setKeyPath:keyPath];
    [self updateTextField];
}

// update the text field to match the editable object
- (void)updateTextField {
    id object = [self object][[self keyPath]];
    if([object isKindOfClass:[NSString class]]) {
        [[self textField] setText:object];
    } else if(object != nil) {
        NSLog(@"TBEditableTextTableViewCell: editable object is not a string");
    }
}

// update the editable object to match the text field
- (void)updateEditableObject {
    [self object][[self keyPath]] = [[self textField] text];
    [[NSNotificationCenter defaultCenter] postNotificationName:kConfigurationUpdatedNotification object:nil];
}

// when selected, make the edit field become the first responder
- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    // don't call super, we don't want to show selection
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
    [super awakeFromNib];
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
    } else if(object != nil) {
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
    [[NSNotificationCenter defaultCenter] postNotificationName:kConfigurationUpdatedNotification object:nil];
}

@end

@interface TBPickableTextTableViewCell () <UIPickerViewDelegate, UIPickerViewDataSource>

// titles for each allowed value
@property (nonatomic, copy) NSArray* allowedValueTitles;

// two-way mapping between objects and titles
@property (nonatomic, retain) NSDictionary* valueForTitle;
@property (nonatomic, retain) NSDictionary* titleForValue;

@end

@implementation TBPickableTextTableViewCell

// synchronize picker selection with text
- (void)updatePickerSelection {
    // get text field's current object value
    NSUInteger selectedIndex = [[self allowedValueTitles] indexOfObject:[[self textField] text]];

    // is current text value allowed? if so, select it, otherwise select first object
    if(selectedIndex == NSNotFound) {
        selectedIndex = 0;
    }

    // get picker
    UIPickerView* picker = (UIPickerView*)[[self textField] inputView];
    [picker selectRow:selectedIndex inComponent:0 animated:NO];
}

// update the text field to match the editable object
- (void)updateTextField {
    NSString* keyPath = [self keyPath];
    if(keyPath == nil) {
        NSLog(@"updateTextField: keypath is nil");
    } else {
        id object = [self object][keyPath];
        if(object == nil) {
            NSLog(@"updateTextField: object is nil");
        } else {
            NSString* text = [self titleForValue][object];
            if(text == nil) {
                NSLog(@"updateTextField: text is nil");
            } else {
                [[self textField] setText:text];

                // sync picker
                [self updatePickerSelection];
            }
        }
    }
}

// update the editable object to match the text field
- (void)updateEditableObject {
    NSString* text = [[self textField] text];
    if(text == nil) {
        NSLog(@"updateEditableObject: text is nil");
    }

    id object = [self valueForTitle][text];
    if(object == nil) {
        NSLog(@"updateEditableObject: object is nil");
    }

    [self object][[self keyPath]] = object;
}

// use a picker instead of free-form text
- (void)setAllowedValues:(NSArray*)allowedValues withTitles:(NSArray*)titles {
    NSParameterAssert(allowedValues);

    // reset picker
    [[self textField] setInputView:nil];
    [[self textField] setInputAccessoryView:nil];

    // build default titles if missing
    if(titles == nil) {
        titles = [allowedValues valueForKey:@"stringValue"];
    }

    // array sizes must match
    NSAssert([allowedValues count] == [titles count], @"setAllowedValues:withTitles: array sizes must match");

    // set state
    [self setAllowedValueTitles:titles];
    [self setValueForTitle:[NSDictionary dictionaryWithObjects:allowedValues forKeys:titles]];
    [self setTitleForValue:[NSDictionary dictionaryWithObjects:titles forKeys:allowedValues]];

    if(allowedValues && titles) {
        // set up a picker
        UIPickerView* picker = [[UIPickerView alloc] init];
        [picker setDataSource:self];
        [picker setDelegate:self];
        [[self textField] setInputView:picker];

        // sync picker with text
        [self updatePickerSelection];

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
    return [[self allowedValueTitles] count];
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
    // make sure object is updated after selection, in case user dismisses dialog while picker is open
    [self updateEditableObject];
}

@end

@implementation TBCheckmarkNumberTableViewCell

// set the object and keypath to observe/sync with accessory check
- (void)setKeyPath:(NSString*)keyPath {
    [super setKeyPath:keyPath];
    if([self object][[self keyPath]] == nil) {
        [self setAccessoryType:UITableViewCellAccessoryNone];
    } else {
        [self setAccessoryType:UITableViewCellAccessoryCheckmark];
    }
}

// when selected, toggle
- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];
    if(selected) {
        if([self accessoryType] == UITableViewCellAccessoryNone) {
            [self object][[self keyPath]] = @0;
            [self setAccessoryType:UITableViewCellAccessoryCheckmark];
        } else {
            [[self object] removeObjectForKey:[self keyPath]];
            [self setAccessoryType:UITableViewCellAccessoryNone];
        }
        [[NSNotificationCenter defaultCenter] postNotificationName:kConfigurationUpdatedNotification object:nil];
    }
}

@end
