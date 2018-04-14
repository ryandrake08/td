//
//  TBAuthCodeViewController.m
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBAuthCodeViewController.h"
#import "TBDateStringTransformer.h"
#import "TBMacDocument.h"

@interface TBAuthCodeViewController ()

// UI Outlets
@property (nonatomic, weak) IBOutlet NSTextField* codeField0;
@property (nonatomic, weak) IBOutlet NSTextField* codeField1;
@property (nonatomic, weak) IBOutlet NSTextField* codeField2;
@property (nonatomic, weak) IBOutlet NSTextField* codeField3;
@property (nonatomic, weak) IBOutlet NSTextField* codeField4;

@end

@implementation TBAuthCodeViewController

#pragma mark Text field navigation

- (void)controlTextDidChange:(NSNotification*)notification {
    id textField = [notification object];

    // chop to one character
    NSString* value = [[textField stringValue] substringToIndex:1];
    NSNumber* numberValue = [[textField formatter] numberFromString:value];

    if(numberValue != nil) {
        // set text field value to validated value
        [textField setStringValue:value];

        // select next field or end if at the last one
        if([textField isEqual:[self codeField4]]) {
            [self doneButtonDidChange:nil];
        } else {
            [[[self view] window] makeFirstResponder:[[notification object] nextValidKeyView]];
        }
    } else {
        [textField setStringValue:nil];
    }
}

#pragma mark Actions

- (IBAction)doneButtonDidChange:(id)sender {
    // parse code
    NSArray* components = @[[[self codeField0] stringValue],[[self codeField1] stringValue],[[self codeField2] stringValue],[[self codeField3] stringValue],[[self codeField4] stringValue]];
    NSString* codeString = [components componentsJoinedByString:@""];
    NSNumberFormatter* formatter = [[NSNumberFormatter alloc] init];
    [formatter setNumberStyle:NSNumberFormatterDecimalStyle];
    NSNumber* code = [formatter numberFromString:codeString];

    // added_at
    TBDateStringTransformer* transformer = [[TBDateStringTransformer alloc] init];
    NSString* addedAt = [transformer reverseTransformedValue:[NSDate date]];

    if(code && addedAt) {
        // get document from sheet parent
        TBMacDocument* document = [[[[[self view] window] sheetParent] windowController] document];

        // send json representation of authorized client to TBMacDocument
        [document addAuthorizedClient:@{@"code":code, @"added_at":addedAt}];
    }

    // dismiss
    [self dismissController:self];
}


@end
