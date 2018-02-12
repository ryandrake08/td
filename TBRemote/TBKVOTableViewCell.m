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
    NSLog(@"DEBUG: setValue:%@ forKeyPath:%@ -> %@", value, [self keyPath], [[self object] valueForKeyPath:[self keyPath]]);

    // tell the world we've edited something
    [[NSNotificationCenter defaultCenter] postNotificationName:kConfigurationUpdatedNotification object:nil];
}

- (NSString*)textRepresentationOfUnderlyingValue {
    id underlyingValue = [self underlyingValue];
    if(underlyingValue == nil) {
        // value is not set for key
        return @" ";
    } else if([underlyingValue respondsToSelector:@selector(count)]) {
        return [NSString stringWithFormat: @"%ld", (long)[underlyingValue count]];
    } else if([underlyingValue respondsToSelector:@selector(stringValue)]) {
        return [underlyingValue stringValue];
    } else if([underlyingValue isKindOfClass:[NSString class]]) {
        return underlyingValue;
    } else {
        NSLog(@"TBKVOTableViewCell: underlyingValue class %@ not supported", [underlyingValue class]);
        return @" ";
    }
    return nil;
}

@end

@implementation TBFormattedKVOTableViewCell

- (NSString*)textRepresentationOfUnderlyingValue {
    if([self formatter] != nil) {
        NSNumber* underlyingNumber = [self underlyingValue];
        if(underlyingNumber == nil) {
            underlyingNumber = @0;
        }
        return [[self formatter] stringFromNumber:underlyingNumber];
    } else {
        return [super textRepresentationOfUnderlyingValue];
    }
}

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

@implementation TBTextFieldTableViewCell

// set the object and use keypath to observe/sync with control
- (void)setObject:(id)object {
    // call base to store the object
    [super setObject:object];

    // set up the textfield
    [[self textField] setText:[self textRepresentationOfUnderlyingValue]];
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

    // get text
    NSString* text = [[self textField] text];

    // set the underlying value
    if([self formatter] != nil) {
        // if there is a formatter, underlying value should be numeric
        [self setUnderlyingValue:[[self formatter] numberFromString:text]];
    } else {
        // otherwise, we assume it is text, warn if it looks numeric
        NSNumberFormatter* testFormatter = [[NSNumberFormatter alloc] init];
        [testFormatter setNumberStyle:NSNumberFormatterDecimalStyle];
        NSNumber* number = [testFormatter numberFromString:text];
        if(number != nil) {
            NSLog(@"TBTextFieldTableViewCell has no formatter, yet a number was entered by user");
        }
        [self setUnderlyingValue:text];
    }
}

@end

@interface TBPickableTextTableViewCell () <UIPickerViewDelegate, UIPickerViewDataSource>

// allowed values
@property (nonatomic, copy) NSArray* allowedValues;

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

- (void)updatePickerSelectionFromTextField {
    NSInteger selectRow = -1;

    // get text field's current object value
    NSString* currentText = [[self textField] text];
    if(![currentText isEqualToString:@""]) {
        NSUInteger selectedIndex = [[self allowedValueTitles] indexOfObject:currentText];

        // is current text value allowed? if so, select it, otherwise select first object
        if(selectedIndex == NSNotFound) {
            NSLog(@"TBPickableTextTableViewCell text entered is not an allowed title");
        } else {
            selectRow = selectedIndex;
        }
    }

    // get picker and select the right row
    UIPickerView* picker = (UIPickerView*)[[self textField] inputView];
    [picker selectRow:selectRow inComponent:0 animated:NO];
}

// set the object and use keypath to observe/sync with control
- (void)setObject:(id)object {
    // call base to store the object
    [super setObject:object];

    [self updatePickerSelectionFromTextField];
}

// use a picker instead of free-form text
- (void)setAllowedValues:(NSArray*)values withTitles:(NSArray*)titles {
    // must have values
    NSParameterAssert(values);

    // build default titles if missing
    if(titles == nil) {
        titles = [values valueForKey:@"stringValue"];
    }

    // array sizes must match
    NSAssert([values count] == [titles count], @"setAllowedValues:withTitles: array sizes must match");

    // reset picker
    [[self textField] setInputView:nil];
    [[self textField] setInputAccessoryView:nil];

    // set state
    [self setAllowedValues:values];
    [self setAllowedValueTitles:titles];

    // create forward and reverse lookups
    [self setValueForTitle:[NSDictionary dictionaryWithObjects:values forKeys:titles]];
    [self setTitleForValue:[NSDictionary dictionaryWithObjects:titles forKeys:values]];

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

    // update selection
    [self updatePickerSelectionFromTextField];
}

#pragma mark UITextFieldDelegate

- (BOOL)textField:(UITextField*)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString*)string {
    // picker should be exclusive source of input for textField
    return NO;
}

- (void)textFieldDidEndEditing:(UITextField*)textField {
    [textField resignFirstResponder];

    // force call to delegate, in case it never happened
    UIPickerView* picker = (UIPickerView*)[[self textField] inputView];
    NSInteger row = [picker selectedRowInComponent:0];
    [self pickerView:picker didSelectRow:row inComponent:0];
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
    if(row < 0) {
        NSLog(@"TBPickableTextTableViewCell: picker deselected");
    } else if(row >= [[self allowedValues] count]) {
        NSLog(@"TBPickableTextTableViewCell: selected row out of bounds");
    } else {
        // set the textfield
        [[self textField] setText:[self allowedValueTitles][row]];

        // set the underlying value
        [self setUnderlyingValue:[self allowedValues][row]];
    }
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
        // TODO: This logic looks incorrect
        if([self accessoryType] == UITableViewCellAccessoryNone) {
            [self setUnderlyingValue:@0];
            [self setAccessoryType:UITableViewCellAccessoryCheckmark];
        } else {
            [[self object] removeObjectForKey:[self keyPath]];
            [self setAccessoryType:UITableViewCellAccessoryNone];
        }
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
