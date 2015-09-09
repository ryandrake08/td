//
//  TBAuthCodeWindowController.m
//  td
//
//  Created by Ryan Drake on 9/3/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBAuthCodeWindowController.h"
#import "TBDateStringTransformer.h"

@interface TBAuthCodeWindowController ()

@property (weak) IBOutlet NSTextField* codeField0;
@property (weak) IBOutlet NSTextField* codeField1;
@property (weak) IBOutlet NSTextField* codeField2;
@property (weak) IBOutlet NSTextField* codeField3;
@property (weak) IBOutlet NSTextField* codeField4;

@end

@implementation TBAuthCodeWindowController

#pragma mark Accessors

// json representation of authorized_client
- (NSDictionary*) object {
    // code
    NSArray* components = @[[[self codeField0] stringValue],[[self codeField1] stringValue],[[self codeField2] stringValue],[[self codeField3] stringValue],[[self codeField4] stringValue]];
    NSString* codeString = [components componentsJoinedByString:@""];
    NSNumberFormatter* formatter = [[NSNumberFormatter alloc] init];
    [formatter setNumberStyle:NSNumberFormatterDecimalStyle];
    NSNumber* code = [formatter numberFromString:codeString];

    // added_at
    TBDateStringTransformer* transformer = [[TBDateStringTransformer alloc] init];
    NSString* addedAt = [transformer reverseTransformedValue:[NSDate date]];

    // build dictionary
    if(code && addedAt) {
        return @{@"code":code, @"added_at":addedAt};
    } else {
        return nil;
    }
}

#pragma mark Text field navigation

- (void)controlTextDidChange:(NSNotification*)notification {
    id textField = [notification object];

    // chop to one character
    NSString* value = [[textField stringValue] substringToIndex:1];
    NSNumber* numberValue = [[textField formatter] numberFromString:value];

    if(numberValue != nil) {
        // Set text field value to validated value
        [textField setStringValue:value];

        // select next field or end if at the last one
        if([textField isEqual:[self codeField4]]) {
            [[[self window] sheetParent] endSheet:[self window] returnCode:NSModalResponseOK];
        } else {
            [[self window] makeFirstResponder:[[notification object] nextValidKeyView]];
        }
    } else {
        [textField setStringValue:nil];
    }
}

#pragma mark Actions

- (IBAction)cancelButtonDidChange:(id)sender {
    [[[self window] sheetParent] endSheet:[self window] returnCode:NSModalResponseCancel];
}

- (IBAction)doneButtonDidChange:(id)sender {
    [[[self window] sheetParent] endSheet:[self window] returnCode:NSModalResponseOK];
}

@end
