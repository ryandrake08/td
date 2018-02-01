//
//  TBTextFieldTableViewCell.m
//  td
//
//  Created by Ryan Drake on 9/25/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBKVOTableViewCell.h"
#import "TBColor+CSS.h"
#import "TBEllipseView.h"
#import "TBNotifications.h"

@implementation TBKVOTableViewCell

- (void)validate {
    if([self object] == nil) {
        NSLog(@"TBKVOTableViewCell: tracked object is nil");
    }
    if([self keyPath] == nil) {
        NSLog(@"TBKVOTableViewCell: keyPath is nil");
    }
}

- (id)underlyingValue {
    [self validate];
    return [[self object] valueForKeyPath:[self keyPath]];
}

- (void)setUnderlyingValue:(id)value {
    [self validate];
    [[self object] setValue:value forKeyPath:[self keyPath]];
}

- (NSString*)textRepresentationOfUnderlyingValue {
    id underlyingValue = [self underlyingValue];
    if([underlyingValue respondsToSelector:@selector(count)]) {
        return [NSString stringWithFormat: @"%ld", (long)[underlyingValue count]];
    } else if([underlyingValue respondsToSelector:@selector(stringValue)]) {
        return [underlyingValue stringValue];
    } else if([underlyingValue respondsToSelector:@selector(count)]) {
        return [NSString stringWithFormat: @"%ld", (long)[underlyingValue count]];
    } else if([underlyingValue isKindOfClass:[NSString class]]) {
        return underlyingValue;
    } else {
        return @" ";
    }
    return nil;
}

- (void)setUnderlyingValueFromTextRepresentation:(NSString*)text {
    id underlyingValue = [self underlyingValue];
    if([underlyingValue isKindOfClass:[NSNumber class]]) {
        [self setUnderlyingValue:[NSNumber numberWithDouble:[text doubleValue]]];
    } else if([underlyingValue isKindOfClass:[NSString class]]) {
        // set directly to text
        [self setUnderlyingValue:text];
    } else {
        NSLog(@"TBTextFieldTableViewCell: underlyingValueRepresentationFromText: underlying value of unsupported class");
    }
}

@end

@implementation TBFormattedKVOTableViewCell

- (NSString*)textRepresentationOfUnderlyingValue {
    if([self formatter] != nil) {
        return [[self formatter] stringFromNumber:[self underlyingValue]];
    } else {
        return [super textRepresentationOfUnderlyingValue];
    }
}

- (void)setUnderlyingValueFromTextRepresentation:(NSString*)text {
    if([self formatter] != nil) {
        [self setUnderlyingValue:[[self formatter] numberFromString:text]];
    } else {
        [super setUnderlyingValueFromTextRepresentation:text];
    }
}

@end

@interface TBLabelTableViewCell ()

// ui outlet
@property (nonatomic, strong) IBOutlet UILabel* label;

@end

@implementation TBLabelTableViewCell

// set the object and use keypath to observe/sync with control
- (void)setObject:(id)object {
    // call base to store the object
    [super setObject:object];

    // set up the textfield
    [[self label] setText:[self textRepresentationOfUnderlyingValue]];
}

@end

@interface TBTextFieldTableViewCell () <UITextFieldDelegate>

// ui outlet
@property (nonatomic, strong) IBOutlet UITextField* textField;

@end

@implementation TBTextFieldTableViewCell

// set the object and use keypath to observe/sync with control
- (void)setObject:(id)object {
    // call base to store the object
    [super setObject:object];

    // set up the textfield
    [[self textField] setText:[self textRepresentationOfUnderlyingValue]];
}

// update the editable object to match the text field
- (void)updateEditableObject {
    // set the underlying value
    [self setUnderlyingValueFromTextRepresentation:[[self textField] text]];

    // tell the world we've edited something
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
    [textField resignFirstResponder];
    [self updateEditableObject];
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

- (NSString*)textRepresentationOfUnderlyingValue {
    return [self titleForValue][[super underlyingValue]];
}

- (void)setUnderlyingValueFromTextRepresentation:(NSString*)text {
    [super setUnderlyingValueFromTextRepresentation:[self valueForTitle][text]];
}

// set the object and use keypath to observe/sync with control
- (void)setObject:(id)object {
    // call base to store the object
    [super setObject:object];

    // get text field's current object value
    NSUInteger selectedIndex = [[self allowedValueTitles] indexOfObject:[[self textField] text]];

    // is current text value allowed? if so, select it, otherwise select first object
    if(selectedIndex == NSNotFound) {
        NSLog(@"TBPickableTextTableViewCell text entered is not an allowed title");
        selectedIndex = 0;
    }

    // get picker
    UIPickerView* picker = (UIPickerView*)[[self textField] inputView];
    [picker selectRow:selectedIndex inComponent:0 animated:NO];
}

// use a picker instead of free-form text
- (void)setAllowedValues:(NSArray*)allowedValues withTitles:(NSArray*)titles {
    // must have allowedValues
    NSParameterAssert(allowedValues);

    // build default titles if missing
    if(titles == nil) {
        titles = [allowedValues valueForKey:@"stringValue"];
    }

    // array sizes must match
    NSAssert([allowedValues count] == [titles count], @"setAllowedValues:withTitles: array sizes must match");

    // reset picker
    [[self textField] setInputView:nil];
    [[self textField] setInputAccessoryView:nil];

    // set state
    [self setAllowedValueTitles:titles];
    [self setValueForTitle:[NSDictionary dictionaryWithObjects:allowedValues forKeys:titles]];
    [self setTitleForValue:[NSDictionary dictionaryWithObjects:titles forKeys:allowedValues]];

    // set up a picker
    UIPickerView* picker = [[UIPickerView alloc] init];
    [picker setDataSource:self];
    [picker setDelegate:self];
    [[self textField] setInputView:picker];

    // set up picker tool bar
    UIToolbar* toolbar = [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, 320, 44)];
    UIBarButtonItem* space = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil];
    UIBarButtonItem* doneButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:[self textField] action:@selector(resignFirstResponder)];
    [toolbar setItems:@[space, doneButton]];
    [[self textField] setInputAccessoryView:toolbar];
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
    // set the textfield
    [[self textField] setText:[self allowedValueTitles][row]];

    // make sure object is updated after selection, in case user dismisses dialog while picker is open
    [self updateEditableObject];
}

@end

@implementation TBCheckmarkNumberTableViewCell

// set the object and use keypath to observe/sync with control
- (void)setObject:(id)object {
    // call base to store the object
    [super setObject:object];

    if([self underlyingValue] == nil) {
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
            [self setUnderlyingValue:@0];
            [self setAccessoryType:UITableViewCellAccessoryCheckmark];
        } else {
            [[self object] removeObjectForKey:[self keyPath]];
            [self setAccessoryType:UITableViewCellAccessoryNone];
        }

        // tell the world we've edited something
        [[NSNotificationCenter defaultCenter] postNotificationName:kConfigurationUpdatedNotification object:nil];
    }
}

@end

@interface TBEllipseTableViewCell ()

// ui outlet
@property (nonatomic, strong) IBOutlet TBEllipseView* ellipseView;

@end

@implementation TBEllipseTableViewCell

// set the object and use keypath to observe/sync with control
- (void)setObject:(id)object {
    // call base to store the object
    [super setObject:object];

    // set up the textfield
    [[self ellipseView] setColor:[TBColor colorWithName:[self textRepresentationOfUnderlyingValue]]];
}

@end
